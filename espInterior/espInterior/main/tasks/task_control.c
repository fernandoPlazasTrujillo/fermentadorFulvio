#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "esp_log.h"

#include "utils/data_types.h"
#include "queues.h"

static const char *TAG = "CONTROL";

#define CO2_THRESHOLD 0.02
#define MIX_INTERVAL_CYCLES 3

void task_control(void *pvParameters)
{
    sensor_data_t data;
    control_cmd_t cmd;

    int cycle_count = 0;

    while (1) {

        if (xQueueReceive(queue_sensores, &data, portMAX_DELAY)) {

            cycle_count++;

            ESP_LOGI(TAG, "Procesando datos...");

            // =====================
            // CO2
            // =====================
            int aire_activo = (data.co2 > CO2_THRESHOLD);

            // =====================
            // MEZCLA CONTROLADA
            // =====================
            cmd.mezclar = (cycle_count % MIX_INTERVAL_CYCLES == 0);

            // =====================
            // ENFRIAMIENTO (placeholder)
            // =====================
            cmd.enfriar = 0;

            cmd.bomba = cmd.enfriar;

            ESP_LOGI(TAG, "Ciclo: %d", cycle_count);
            ESP_LOGI(TAG, "Mezclar: %d", cmd.mezclar);

            xQueueSend(queue_control, &cmd, portMAX_DELAY);
        }
    }
}