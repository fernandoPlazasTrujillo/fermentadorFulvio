#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "common.h"
#include "mqtt/mqtt_manager.h"
#include "queues.h"

#define MQTT_TOPIC "fermentador/espExterior/datos"
#define MQTT_CONNECT_TIMEOUT_MS 8000
#define MQTT_PUBLISH_TIMEOUT_MS 5000

extern TaskHandle_t main_task_handle;

void task_mqtt(void *pvParameters)
{
    datos_ambiente_t datos;
    char payload[192];

    vTaskDelay(pdMS_TO_TICKS(3000));

    mqtt_manager_start();

    if (!mqtt_wait_connected(MQTT_CONNECT_TIMEOUT_MS))
    {
        printf("MQTT no conectado\n");

        mqtt_manager_stop();

        xTaskNotifyGive(main_task_handle);
        
        vTaskDelete(NULL);
    }

    if (xQueueReceive(cola_mqtt, &datos, portMAX_DELAY))
    {
        snprintf(payload, sizeof(payload),
                 "{\"fecha\":\"%02d-%02d-%04d\",\"hora\":\"%02d.%02d.%02d\",\"temperatura\":%.2f,\"humedad\":%.2f}",
                 datos.dia,
                 datos.mes,
                 datos.anio,
                 datos.hora,
                 datos.minuto,
                 datos.segundo,
                 datos.temperatura,
                 datos.humedad);

        printf("MQTT payload -> %s\n", payload);

        if (mqtt_publish(MQTT_TOPIC, payload, 1, MQTT_PUBLISH_TIMEOUT_MS))
        {
            printf("Dato publicado por MQTT\n");
        }
        else
        {
            printf("Error publicando por MQTT\n");
        }
    }

    mqtt_manager_stop();
    xTaskNotifyGive(main_task_handle);
    vTaskDelete(NULL);
}
