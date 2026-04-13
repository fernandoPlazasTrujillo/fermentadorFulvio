/**
 * @file mq135.c
 * @brief Implementación del driver para el sensor MQ135.
 * 
 * Este módulo permite leer valores de calidad del aire mediante
 * el uso del ADC del ESP32.
 * 
 * @author
 * Fernando Plazas Trujillo
 * Isabella Ordoñez
 * Juan Daniel Constain
 */

#include "mq135.h"
#include "hal/adc_manager.h"
#include "esp_log.h"

#define TAG "MQ135"

/** @brief Canal ADC utilizado (GPIO36) */
#define MQ135_ADC_CHANNEL ADC_CHANNEL_0  

/** @brief Número de muestras para promediado */
#define NUM_SAMPLES 10

// =====================
// INIT
// =====================

/**
 * @brief Inicializa el sensor MQ135.
 */
void mq135_init(void)
{
    ESP_LOGI(TAG, "MQ135 inicializado");
}

// =====================
// READ RAW (ADC)
// =====================

/**
 * @brief Obtiene un valor crudo estimado del ADC.
 * 
 * Convierte el voltaje leído a una escala de 12 bits (0–4095).
 * 
 * @return Valor ADC estimado.
 */
int mq135_read_raw(void)
{
    int raw = 0;

    float voltage = adc_manager_read_voltage(MQ135_ADC_CHANNEL);

    raw = (int)((voltage / 3.3) * 4095);

    ESP_LOGI(TAG, "RAW estimado: %d", raw);

    return raw;
}

// =====================
// READ VOLTAGE
// =====================

/**
 * @brief Lee el voltaje promedio del sensor.
 * 
 * Realiza múltiples lecturas para reducir ruido.
 * 
 * @return Voltaje promedio en voltios.
 */
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
// READ (GENERAL)
// =====================

/**
 * @brief Función principal de lectura del sensor MQ135.
 * 
 * @return Voltaje del sensor.
 */
float mq135_read(void)
{
    return mq135_read_voltage();
}