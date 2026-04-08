#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_sleep.h"

extern TaskHandle_t task_sensores_handle;

static const char *TAG = "ENERGIA";

void task_energia(void *pvParameters)
{
    while (1) {

        // Inicio ciclo
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        ESP_LOGI(TAG, "Ciclo iniciado");

        // activar sensores
        xTaskNotifyGive(task_sensores_handle);

        // 🔥 esperar actuadores
        ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(25000));

        // 🔥 esperar logger (NUEVO)
        ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(5000));

        ESP_LOGI(TAG, "Entrando en deep sleep");

        esp_deep_sleep_start();
    }
}