#include "voltage_sensor.h"
#include "hal/adc_manager.h"
#include "esp_log.h"

#define TAG "VOLTAGE"

#define VOLTAGE_ADC_CHANNEL ADC_CHANNEL_4  // GPIO32
#define DIVIDER_RATIO 5.0
#define NUM_SAMPLES 10

// Ajusta este valor según tu offset real
#define ADC_OFFSET 0.48

void voltage_sensor_init(void)
{
    ESP_LOGI(TAG, "Voltage sensor inicializado");
}

float voltage_sensor_read(void)
{
    float sum = 0;

    for (int i = 0; i < NUM_SAMPLES; i++) {
        sum += adc_manager_read_voltage(VOLTAGE_ADC_CHANNEL);
    }

    float v_adc = sum / NUM_SAMPLES;

    // Compensación
    v_adc -= ADC_OFFSET;
    if (v_adc < 0) v_adc = 0;

    float v_real = v_adc * DIVIDER_RATIO;

    ESP_LOGI(TAG, "Vadc corregido: %.3f V", v_adc);
    ESP_LOGI(TAG, "Voltaje final: %.2f V", v_real);

    return v_real;
}