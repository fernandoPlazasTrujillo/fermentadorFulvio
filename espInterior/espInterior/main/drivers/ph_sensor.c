#include "ph_sensor.h"
#include "hal/adc_manager.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#define PH_ADC_CHANNEL ADC_CHANNEL_3  // GPIO39
#define NUM_SAMPLES 20

#define PH_SLOPE   -5.7
#define PH_OFFSET   21.34

static const char *TAG = "PH";

void ph_init(void)
{
}

float ph_read_voltage(void)
{
    float sum = 0;

    for (int i = 0; i < NUM_SAMPLES; i++) {
        sum += adc_manager_read_voltage(PH_ADC_CHANNEL);
        vTaskDelay(pdMS_TO_TICKS(10));
    }

    return sum / NUM_SAMPLES;
}

float ph_read(void)
{
    float voltage = ph_read_voltage();

    if (voltage > 3.2 || voltage < 0.1) {
        ESP_LOGW(TAG, "Sensor pH fuera de rango");
        return -1.0;
    }

    return PH_SLOPE * voltage + PH_OFFSET;
}
