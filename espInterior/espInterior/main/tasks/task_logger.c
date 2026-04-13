/**
 * @file task_sensores.c
 * @brief Adquisición y procesamiento de sensores.
 * 
 * Lee sensores, aplica filtrado y calcula variables derivadas.
 * Envía los datos a las colas del sistema.
 */

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

RTC_DATA_ATTR static float baseline = 0;
RTC_DATA_ATTR static int baseline_initialized = 0;

#define ALPHA 0.2

static float filtered = 0;

/**
 * @brief Tarea de sensores.
 */
void task_sensores(void *pvParameters)
{
    sensor_data_t data;

    mq135_init();
    ph_init();

    ds18b20_init(GPIO_NUM_4);

    while (1) {

        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        ESP_LOGI(TAG, "Leyendo sensores...");

        float raw_voltage = mq135_read();
        int raw_adc = mq135_read_raw();

        ESP_LOGI(TAG, "MQ135 ADC RAW: %d", raw_adc);

        filtered = ALPHA * raw_voltage + (1 - ALPHA) * filtered;

        if (!baseline_initialized) {
            baseline = filtered;
            baseline_initialized = 1;
            ESP_LOGI(TAG, "Baseline inicial: %.4f", baseline);
        }

        data.co2_raw = raw_voltage;
        data.co2 = filtered - baseline;

        data.ph = ph_read();

        data.voltaje = 5.0;

        float temp = ds18b20_read_temperature();

        if (temp == -127.0) {
            ESP_LOGW(TAG, "Error leyendo DS18B20");
            temp = 25.0;
        }

        data.temperatura = temp;
        data.corriente = 0.5;

        ds3231_get_datetime(&data.datetime);

        xQueueSend(queue_sensores, &data, portMAX_DELAY);
        xQueueSend(queue_logger, &data, portMAX_DELAY);

        ESP_LOGI(TAG, "Datos enviados");
    }
}