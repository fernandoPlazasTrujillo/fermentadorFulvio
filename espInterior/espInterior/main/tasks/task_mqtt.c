#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "drivers/sd_card.h"
#include "mqtt/mqtt_manager.h"
#include "queues.h"
#include "utils/data_types.h"

#define MQTT_TOPIC "fermentador/espInterior/datos"
#define MQTT_CONNECT_TIMEOUT_MS 8000
#define MQTT_PUBLISH_TIMEOUT_MS 5000
#define MQTT_PENDING_PAYLOAD_SIZE 320

extern TaskHandle_t task_energia_handle;

static void save_pending_payload(const char *payload)
{
    if (sd_card_append_pending_mqtt(payload) == ESP_OK)
    {
        printf("Payload MQTT guardado en pendientes\n");
    }
    else
    {
        printf("Error guardando payload MQTT pendiente\n");
    }
}

static bool resend_pending_mqtt_messages(void)
{
    char pending_payload[MQTT_PENDING_PAYLOAD_SIZE];
    FILE *pending_file;

    if (!sd_card_pending_mqtt_exists())
    {
        return true;
    }

    printf("Reenviando mensajes MQTT pendientes desde SD\n");

    pending_file = sd_card_open_pending_mqtt_read();
    if (pending_file == NULL)
    {
        return false;
    }

    while (sd_card_read_pending_mqtt_line(pending_file,
                                          pending_payload,
                                          sizeof(pending_payload)) != NULL)
    {
        pending_payload[strcspn(pending_payload, "\r\n")] = '\0';

        if (pending_payload[0] == '\0')
        {
            continue;
        }

        printf("MQTT pendiente -> %s\n", pending_payload);

        if (!mqtt_publish(MQTT_TOPIC,
                          pending_payload,
                          1,
                          MQTT_PUBLISH_TIMEOUT_MS))
        {
            printf("Error reenviando payload MQTT pendiente\n");
            sd_card_close_pending_mqtt(pending_file);
            return false;
        }
    }

    sd_card_close_pending_mqtt(pending_file);

    if (sd_card_delete_pending_mqtt() != ESP_OK)
    {
        printf("Mensajes pendientes reenviados, pero no se pudo eliminar el archivo\n");
        return false;
    }

    printf("Mensajes MQTT pendientes reenviados correctamente\n");

    return true;
}

static void build_payload(const sensor_data_t *data, char *payload, size_t payload_size)
{
    snprintf(payload, payload_size,
             "{\"timestamp\":\"%04d-%02d-%02d %02d:%02d:%02d\","
             "\"temperatura\":%.2f,\"ph\":%.2f,\"co2\":%.4f,"
             "\"co2_raw\":%.4f,\"voltaje\":%.2f,\"corriente\":%.2f}",
             data->datetime.year,
             data->datetime.month,
             data->datetime.day,
             data->datetime.hours,
             data->datetime.minutes,
             data->datetime.seconds,
             data->temperatura,
             data->ph,
             data->co2,
             data->co2_raw,
             data->voltaje,
             data->corriente);
}

void task_mqtt(void *pvParameters)
{
    sensor_data_t data;
    char payload[MQTT_PENDING_PAYLOAD_SIZE];

    while (1)
    {
        if (xQueueReceive(queue_mqtt, &data, portMAX_DELAY))
        {
            build_payload(&data, payload, sizeof(payload));

            printf("MQTT payload -> %s\n", payload);

            mqtt_manager_start();

            if (!mqtt_wait_connected(MQTT_CONNECT_TIMEOUT_MS))
            {
                printf("MQTT no conectado\n");
                save_pending_payload(payload);
                mqtt_manager_stop();
                xTaskNotifyGive(task_energia_handle);
                continue;
            }

            if (!resend_pending_mqtt_messages())
            {
                printf("No se pudieron reenviar todos los pendientes MQTT\n");
                save_pending_payload(payload);
                mqtt_manager_stop();
                xTaskNotifyGive(task_energia_handle);
                continue;
            }

            if (mqtt_publish(MQTT_TOPIC, payload, 1, MQTT_PUBLISH_TIMEOUT_MS))
            {
                printf("Dato publicado por MQTT\n");
            }
            else
            {
                printf("Error publicando por MQTT\n");
                save_pending_payload(payload);
            }

            mqtt_manager_stop();
            xTaskNotifyGive(task_energia_handle);
        }
    }
}
