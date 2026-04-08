#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"

#include "utils/data_types.h"
#include "drivers/rtc_ds3231.h"
#include "drivers/mq135.h"
#include "drivers/ph_sensor.h"
#include "drivers/ds18b20.h"
#include "queues.h"

static const char *TAG = "SENSORES";

// =====================
// PERSISTENCIA EN DEEP SLEEP
// =====================
RTC_DATA_ATTR static float baseline = 0;
RTC_DATA_ATTR static int baseline_initialized = 0;

// =====================
// FILTRO EXPONENCIAL
// =====================
#define ALPHA 0.2
static float filtered = 0;

void task_sensores(void *pvParameters)
{
    sensor_data_t data;

    // =====================
    // INIT DRIVERS
    // =====================
    mq135_init();
    ph_init();

    ds18b20_init(GPIO_NUM_4);

    while (1) {

        // =====================
        // ESPERAR ACTIVACIÓN
        // =====================
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        ESP_LOGI(TAG, "Leyendo sensores...");

        // =====================
        // MQ135
        // =====================
        float raw_voltage = mq135_read();
        int raw_adc = mq135_read_raw();

        ESP_LOGI(TAG, "MQ135 ADC RAW: %d", raw_adc);

        // FILTRO EXPONENCIAL
        filtered = ALPHA * raw_voltage + (1 - ALPHA) * filtered;

        // BASELINE INICIAL
        if (!baseline_initialized) {
            baseline = filtered;
            baseline_initialized = 1;
            ESP_LOGI(TAG, "Baseline inicial: %.4f", baseline);
        }

        data.co2_raw = raw_voltage;
        data.co2 = filtered - baseline;

        // =====================
        // pH
        // =====================
        data.ph = ph_read();

        // =====================
        // VALORES FIJOS (DEBUG)
        // =====================
        data.voltaje = 5.0;       // 🔥 fijo
        
        float temp = ds18b20_read_temperature();

        if (temp == -127.0) {
            ESP_LOGW(TAG, "Error leyendo DS18B20");
            temp = 25.0; // fallback
        }

        data.temperatura = temp;
        data.corriente = 0.5;     // puedes dejarlo o también fijarlo luego

        // =====================
        // RTC
        // =====================
        ds3231_get_datetime(&data.datetime);

        // =====================
        // ENVÍO A SISTEMA
        // =====================
        xQueueSend(queue_sensores, &data, portMAX_DELAY);
        xQueueSend(queue_logger, &data, portMAX_DELAY);

        ESP_LOGI(TAG, "Datos enviados");
    }
}