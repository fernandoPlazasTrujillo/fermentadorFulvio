/**
 * @file task_logger.c
 * @brief Registro de datos de sensores en tarjeta SD.
 */

#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "esp_log.h"

#include "utils/data_types.h"
#include "drivers/sd_card.h"
#include "queues.h"

static const char *TAG = "LOGGER";

/**
 * @brief Tarea de logging del sistema.
 */
void task_logger(void *pvParameters)
{
    sensor_data_t data;
    char line[160];

    if (sd_card_init() != ESP_OK) {
        ESP_LOGE(TAG, "SD no inicializada");
    }

    while (1) {
        if (xQueueReceive(queue_logger, &data, portMAX_DELAY)) {
            snprintf(line, sizeof(line),
                     "%02d-%02d-%04d, %02d.%02d.%02d, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f",
                     data.datetime.day,
                     data.datetime.month,
                     data.datetime.year,
                     data.datetime.hours,
                     data.datetime.minutes,
                     data.datetime.seconds,
                     data.temperatura,
                     data.ph,
                     data.co2,
                     data.co2_raw,
                     data.voltaje,
                     data.corriente);

            ESP_LOGI(TAG, "LOG -> %s", line);

            if (sd_card_write_line(line) != ESP_OK) {
                ESP_LOGE(TAG, "Error escribiendo en SD");
            } else {
                ESP_LOGI(TAG, "Dato guardado en SD");
            }
        }
    }
}
