#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "esp_log.h"

#include "utils/data_types.h"
#include "queues.h"
#include "drivers/sd_card.h"

// 🔥 sincronización con energía
extern TaskHandle_t task_energia_handle;

static const char *TAG = "LOGGER";

#define MQ135_THRESHOLD 0.02

void task_logger(void *pvParameters)
{
    sensor_data_t data;
    char line[128];

    vTaskDelay(pdMS_TO_TICKS(500));

    // =====================
    // INIT SD CON REINTENTOS
    // =====================
    int retries = 3;
    esp_err_t ret = ESP_FAIL;

    while (retries--) {

        ret = sd_card_init();

        if (ret == ESP_OK) {
            ESP_LOGI(TAG, "SD montada correctamente");
            break;
        }

        ESP_LOGW(TAG, "Fallo SD, reintentando...");
        vTaskDelay(pdMS_TO_TICKS(500));
    }

    // =====================
    // FALLA CRÍTICA SD
    // =====================
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "SD FALLÓ DEFINITIVO");

        while (1) {
            vTaskDelay(pdMS_TO_TICKS(2000));
        }
    }

    ESP_LOGI(TAG, "Logger listo");

    // =====================
    // LOOP PRINCIPAL
    // =====================
    while (1) {

        if (xQueueReceive(queue_logger, &data, portMAX_DELAY)) {

            // =====================
            // FORMATO CSV
            // =====================
            snprintf(line, sizeof(line),
                "%04d-%02d-%02d %02d:%02d:%02d,%.2f,%.2f,%.4f,%.4f,%.2f,%.2f",
                data.datetime.year,
                data.datetime.month,
                data.datetime.day,
                data.datetime.hours,
                data.datetime.minutes,
                data.datetime.seconds,
                data.temperatura,
                data.ph,
                data.co2_raw,
                data.co2,
                data.voltaje,
                data.corriente
            );

            // =====================
            // ESCRITURA SD
            // =====================
            if (sd_card_write_line(line) != ESP_OK) {
                ESP_LOGE(TAG, "Error escribiendo en SD");
            }

            // =====================
            // DEBUG CONTROLADO
            // =====================
            ESP_LOGI(TAG, "-------------------------");

            ESP_LOGI(TAG, "Fecha: %04d-%02d-%02d",
                data.datetime.year,
                data.datetime.month,
                data.datetime.day);

            ESP_LOGI(TAG, "Hora: %02d:%02d:%02d",
                data.datetime.hours,
                data.datetime.minutes,
                data.datetime.seconds);

            ESP_LOGI(TAG, "Temp: %.2f C", data.temperatura);

            if (data.ph < 0) {
                ESP_LOGI(TAG, "pH: INVALIDO");
            } else {
                ESP_LOGI(TAG, "pH: %.2f", data.ph);
            }

            ESP_LOGI(TAG, "MQ135 raw: %.4f V", data.co2_raw);
            ESP_LOGI(TAG, "MQ135 delta: %.4f V", data.co2);

            if (data.co2 > MQ135_THRESHOLD) {
                ESP_LOGI(TAG, "CO2 detectado: SI");
            } else {
                ESP_LOGI(TAG, "CO2 detectado: NO");
            }

            ESP_LOGI(TAG, "Voltaje: %.2f V", data.voltaje);
            ESP_LOGI(TAG, "Corriente: %.2f A", data.corriente);

            // =====================
            // 🔥 NOTIFICAR A ENERGÍA
            // =====================
            xTaskNotifyGive(task_energia_handle);
        }
    }
}