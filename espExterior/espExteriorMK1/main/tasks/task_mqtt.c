#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "common.h"
#include "mqtt/mqtt_manager.h"
#include "queues.h"

#include "sd_card.h"

#define MQTT_TOPIC "fermentador/espExterior/datos"
#define MQTT_CONNECT_TIMEOUT_MS 8000
#define MQTT_PUBLISH_TIMEOUT_MS 5000

extern TaskHandle_t main_task_handle;

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

            if (sd_append_pending_mqtt(payload) == 0)
            {
                printf("Payload MQTT guardado en pendientes\n");
            }
            else
            {
                printf("Error guardando payload MQTT pendiente\n");
            }

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

            if (sd_append_pending_mqtt(payload) == 0)
                {
                    printf("Payload MQTT guardado en pendientes\n");
                }
                else
                {
                    printf("Error guardando payload MQTT pendiente\n");
                }
        }
    }

    mqtt_manager_stop();
    xTaskNotifyGive(main_task_handle);
    vTaskDelete(NULL);
}
