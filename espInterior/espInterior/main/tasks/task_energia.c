/**
 * @file task_energia.c
 * @brief Gestión energética del sistema.
 * 
 * Coordina la ejecución de sensores, actuadores y logger.
 * Controla el ciclo de operación y entrada a deep sleep.
 */

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_sleep.h"

extern TaskHandle_t task_sensores_handle;

static const char *TAG = "ENERGIA";

/**
 * @brief Tarea de energía.
 */
void task_energia(void *pvParameters)
{
    while (1) {

        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        ESP_LOGI(TAG, "Ciclo iniciado");

        xTaskNotifyGive(task_sensores_handle);

        ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(25000));
        ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(5000));

        ESP_LOGI(TAG, "Entrando en deep sleep");

        esp_deep_sleep_start();
    }
}