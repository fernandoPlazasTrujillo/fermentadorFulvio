/**
 * @file task_control.c
 * @brief Lógica de control del sistema.
 * 
 * Procesa los datos provenientes de sensores y genera comandos
 * para los actuadores.
 */

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "esp_log.h"

#include "utils/data_types.h"
#include "queues.h"

static const char *TAG = "CONTROL";

#define CO2_THRESHOLD 0.02
#define MIX_INTERVAL_CYCLES 3

/**
 * @brief Tarea de control.
 */
void task_control(void *pvParameters)
{
    sensor_data_t data;
    control_cmd_t cmd;

    int cycle_count = 0;

    while (1) {

        if (xQueueReceive(queue_sensores, &data, portMAX_DELAY)) {

            cycle_count++;

            ESP_LOGI(TAG, "Procesando datos...");

            int aire_activo = (data.co2 > CO2_THRESHOLD);

            cmd.mezclar = (cycle_count % MIX_INTERVAL_CYCLES == 0);
            cmd.enfriar = 0;
            cmd.bomba = cmd.enfriar;

            ESP_LOGI(TAG, "Ciclo: %d", cycle_count);
            ESP_LOGI(TAG, "Mezclar: %d", cmd.mezclar);

            xQueueSend(queue_control, &cmd, portMAX_DELAY);
        }
    }
}