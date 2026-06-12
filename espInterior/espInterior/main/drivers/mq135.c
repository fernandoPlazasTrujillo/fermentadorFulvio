/**
 * @file mq135.c
 * @brief Implementación del driver para adquisición de señales del sensor MQ135.
 *
 * Este módulo permite obtener mediciones analógicas provenientes
 * del sensor MQ135 utilizando el subsistema ADC del ESP32.
 *
 * Funcionalidades:
 * - Lectura de voltaje.
 * - Obtención de valores ADC estimados.
 * - Promediado de muestras para reducción de ruido.
 *
 * La conversión a concentraciones específicas de gases no forma
 * parte de este módulo.
 *
 * @author
 * Fernando Plazas Trujillo
 * Isabella Ordoñez
 * Juan Daniel Constain
 */

#include "mq135.h"
#include "hal/adc_manager.h"
#include "esp_log.h"

/**
 * @brief Etiqueta utilizada por el sistema de logging.
 */
#define TAG "MQ135"

/**
 * @brief Canal ADC asociado al sensor MQ135.
 *
 * Corresponde al GPIO36 del ESP32.
 */
#define MQ135_ADC_CHANNEL ADC_CHANNEL_0  

/**
 * @brief Cantidad de muestras utilizadas para el cálculo del promedio.
 *
 * El promediado reduce el efecto del ruido presente en la señal analógica.
 */
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