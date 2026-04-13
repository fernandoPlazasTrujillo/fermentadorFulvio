/**
 * @file voltage_sensor.c
 * @brief Implementación del sensor de voltaje.
 * 
 * Este módulo lee el voltaje desde un ADC, aplica correcciones
 * de offset y calcula el voltaje real mediante un divisor resistivo.
 * 
 * @author
 * Fernando Plazas Trujillo
 * Isabella Ordoñez
 * Juan Daniel Constain
 */

#include "voltage_sensor.h"
#include "hal/adc_manager.h"
#include "esp_log.h"

#define TAG "VOLTAGE"

/** @brief Canal ADC utilizado (GPIO32) */
#define VOLTAGE_ADC_CHANNEL ADC_CHANNEL_4  

/** @brief Relación del divisor de voltaje */
#define DIVIDER_RATIO 5.0

/** @brief Número de muestras para promediado */
#define NUM_SAMPLES 10

/** @brief Offset del ADC para calibración */
#define ADC_OFFSET 0.48

// =====================
// INIT
// =====================

/**
 * @brief Inicializa el sensor de voltaje.
 */
void voltage_sensor_init(void)
{
    ESP_LOGI(TAG, "Voltage sensor inicializado");
}

// =====================
// READ VOLTAGE
// =====================

/**
 * @brief Lee el voltaje real del sistema.
 * 
 * Realiza:
 * - Promedio de muestras
 * - Corrección de offset
 * - Escalado por divisor resistivo
 * 
 * @return Voltaje en voltios.
 */
float voltage_sensor_read(void)
{
    float sum = 0;

    for (int i = 0; i < NUM_SAMPLES; i++) {
        sum += adc_manager_read_voltage(VOLTAGE_ADC_CHANNEL);
    }

    float v_adc = sum / NUM_SAMPLES;

    v_adc -= ADC_OFFSET;
    if (v_adc < 0) v_adc = 0;

    float v_real = v_adc * DIVIDER_RATIO;

    ESP_LOGI(TAG, "Vadc corregido: %.3f V", v_adc);
    ESP_LOGI(TAG, "Voltaje final: %.2f V", v_real);

    return v_real;
}