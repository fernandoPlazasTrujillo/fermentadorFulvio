#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "common.h"
#include "mqtt/mqtt_manager.h"
#include "queues.h"

#include "sd_card.h"

#define MQTT_TOPIC "fermentador/espExterior/datos"
#define MQTT_CONNECT_TIMEOUT_MS 8000
#define MQTT_PUBLISH_TIMEOUT_MS 5000
#define MQTT_PENDING_PAYLOAD_SIZE 256

extern TaskHandle_t main_task_handle;

static void save_pending_payload(const char *payload)
{
    if (sd_append_pending_mqtt(payload) == 0)
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

    if (!sd_pending_mqtt_exists())
    {
        return true;
    }

    printf("Reenviando mensajes MQTT pendientes desde SD\n");

    pending_file = sd_open_pending_mqtt_read();
    if (pending_file == NULL)
    {
        return false;
    }

    while (sd_read_pending_mqtt_line(pending_file,
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
            sd_close_pending_mqtt(pending_file);
            return false;
        }
    }

    sd_close_pending_mqtt(pending_file);

    if (sd_delete_pending_mqtt() != 0)
    {
        printf("Mensajes pendientes reenviados, pero no se pudo eliminar test.txt\n");
        return false;
    }

    printf("Mensajes MQTT pendientes reenviados correctamente\n");

    return true;
}

void task_mqtt(void *pvParameters)
{
    datos_ambiente_t datos;
    char payload[192];

    vTaskDelay(pdMS_TO_TICKS(3000));

    if (xQueueReceive(cola_mqtt, &datos, portMAX_DELAY))
    {
        snprintf(payload, sizeof(payload),
            "{\"timestamp\":\"%04d-%02d-%02d %02d:%02d:%02d\",\"temp\":%.2f,\"hum\":%.2f}",
            datos.anio,
            datos.mes,
            datos.dia,
            datos.hora,
            datos.minuto,
            datos.segundo,
            datos.temperatura,
            datos.humedad);

        printf("MQTT payload -> %s\n", payload);

        mqtt_manager_start();

        if (!mqtt_wait_connected(MQTT_CONNECT_TIMEOUT_MS))
        {
            printf("MQTT no conectado\n");

            save_pending_payload(payload);

            mqtt_manager_stop();

            xTaskNotifyGive(main_task_handle);

            vTaskDelete(NULL);
        }

        if (!resend_pending_mqtt_messages())
        {
            printf("No se pudieron reenviar todos los pendientes MQTT\n");
            save_pending_payload(payload);

            mqtt_manager_stop();
            xTaskNotifyGive(main_task_handle);
            vTaskDelete(NULL);
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
    }

    mqtt_manager_stop();
    xTaskNotifyGive(main_task_handle);
    vTaskDelete(NULL);
}
