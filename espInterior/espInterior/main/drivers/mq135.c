#include "mq135.h"
#include "hal/adc_manager.h"
#include "esp_log.h"

#define TAG "MQ135"

#define MQ135_ADC_CHANNEL ADC_CHANNEL_0  // GPIO36
#define NUM_SAMPLES 10

// =====================
// INIT
// =====================
void mq135_init(void)
{
    ESP_LOGI(TAG, "MQ135 inicializado");
}

// =====================
// READ RAW (ADC)
// =====================
int mq135_read_raw(void)
{
    int raw = 0;

    // Para obtener raw real, hacemos una sola lectura interna
    // (nota: adc_manager devuelve voltaje, así que aquí hacemos aproximación)

    float voltage = adc_manager_read_voltage(MQ135_ADC_CHANNEL);

    // Convertimos a "raw estimado"
    raw = (int)((voltage / 3.3) * 4095);

    ESP_LOGI(TAG, "RAW estimado: %d", raw);

    return raw;
}

// =====================
// READ VOLTAGE
// =====================
float mq135_read_voltage(void)
{
    float sum = 0;

    for (int i = 0; i < NUM_SAMPLES; i++) {
        sum += adc_manager_read_voltage(MQ135_ADC_CHANNEL);
    }

    float avg = sum / NUM_SAMPLES;

    ESP_LOGI(TAG, "Voltaje: %.4f V", avg);

    return avg;
}

// =====================
// READ (COMPATIBLE CON TU SISTEMA)
// =====================
float mq135_read(void)
{
    return mq135_read_voltage();
}