/**
 * @file voltage_sensor.c
 * @brief Implementación del módulo de medición de voltaje.
 *
 * Este módulo obtiene mediciones de voltaje mediante el ADC
 * del ESP32, aplica compensaciones de calibración y calcula
 * el voltaje real utilizando la relación del divisor resistivo.
 *
 * Funcionalidades:
 * - Lectura ADC mediante la capa HAL.
 * - Promediado de muestras.
 * - Compensación de offset.
 * - Conversión a voltaje real.
 *
 * El módulo es utilizado para monitorear el estado de la batería
 * y apoyar las decisiones de gestión energética del sistema.
 *
 * @author
 * Fernando Plazas Trujillo
 * Isabella Ordoñez
 * Juan Daniel Constain
 */

#include "voltage_sensor.h"
#include "hal/adc_manager.h"
#include "esp_log.h"

/**
 * @brief Etiqueta utilizada por el sistema de logging.
 */
#define TAG "VOLTAGE"

/**
 * @brief Canal ADC asociado al sensor de voltaje.
 *
 * Corresponde al GPIO32 del ESP32.
 */
#define VOLTAGE_ADC_CHANNEL ADC_CHANNEL_4

/**
 * @brief Relación del divisor resistivo.
 *
 * Voltaje real = Voltaje ADC × DIVIDER_RATIO
 */
#define DIVIDER_RATIO 5.0f

/**
 * @brief Cantidad de muestras utilizadas para el promedio.
 *
 * El promediado reduce el efecto del ruido presente
 * en la señal analógica.
 */
#define NUM_SAMPLES 10

/**
 * @brief Corrección de offset experimental del ADC.
 *
 * Este valor compensa errores sistemáticos observados
 * durante la calibración del sistema.
 */
#define ADC_OFFSET 0.48f

// =====================
// INIT
// =====================

/**
 * @brief Inicializa el módulo de medición de voltaje.
 *
 * Actualmente no requiere configuración específica,
 * ya que la adquisición ADC es gestionada por adc_manager.
 */
void voltage_sensor_init(void)
{
    ESP_LOGI(TAG, "Voltage sensor inicializado");
}

// =====================
// READ VOLTAGE
// =====================

/**
 * @brief Obtiene el voltaje real del sistema.
 *
 * El procedimiento de medición consiste en:
 * - Adquirir múltiples muestras ADC.
 * - Calcular un promedio.
 * - Aplicar corrección de offset.
 * - Compensar el divisor resistivo.
 *
 * @return Voltaje real medido en voltios.
 */
float voltage_sensor_read(void)
{
    float sum = 0.0f;

    /*
     * Promediado simple para reducir ruido.
     */
    for (int i = 0; i < NUM_SAMPLES; i++) {
        sum += adc_manager_read_voltage(VOLTAGE_ADC_CHANNEL);
    }

    float v_adc = sum / NUM_SAMPLES;

    /*
     * Corrección de offset obtenida durante
     * la calibración experimental.
     */
    v_adc -= ADC_OFFSET;

    if (v_adc < 0.0f) {
        v_adc = 0.0f;
    }

    /*
     * Conversión del voltaje medido en el ADC
     * al voltaje real mediante el divisor resistivo.
     */
    float v_real = v_adc * DIVIDER_RATIO;

    ESP_LOGI(TAG, "Vadc corregido: %.3f V", v_adc);
    ESP_LOGI(TAG, "Voltaje final: %.2f V", v_real);

    return v_real;
}