/**
 * @file ph_sensor.c
 * @brief Implementación del sensor de pH.
 * 
 * Este módulo convierte lecturas analógicas en valores de pH
 * utilizando una ecuación de calibración lineal.
 * 
 * @author
 * Fernando Plazas Trujillo
 * Isabella Ordoñez
 * Juan Daniel Constain
 */

#include "ph_sensor.h"
#include "hal/adc_manager.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

/** @brief Canal ADC utilizado (GPIO39) */
#define PH_ADC_CHANNEL ADC_CHANNEL_3  

/** @brief Número de muestras para promediado */
#define NUM_SAMPLES 20

/** @brief Pendiente de calibración del sensor */
#define PH_SLOPE   -5.7

/** @brief Offset de calibración del sensor */
#define PH_OFFSET   21.34

static const char *TAG = "PH";

// =====================
// INIT
// =====================

/**
 * @brief Inicializa el sensor de pH.
 */
void ph_init(void)
{
}

// =====================
// READ VOLTAGE
// =====================

/**
 * @brief Obtiene el voltaje promedio del sensor de pH.
 * 
 * @return Voltaje en voltios.
 */
float ph_read_voltage(void)
{
    float sum = 0;

    for (int i = 0; i < NUM_SAMPLES; i++) {
        sum += adc_manager_read_voltage(PH_ADC_CHANNEL);
        vTaskDelay(pdMS_TO_TICKS(10));
    }

    return sum / NUM_SAMPLES;
}

// =====================
// READ PH
// =====================

/**
 * @brief Calcula el valor de pH a partir del voltaje medido.
 * 
 * Usa una relación lineal calibrada:
 * pH = slope * V + offset
 * 
 * @return Valor de pH.
 * @return -1.0 si el sensor está fuera de rango.
 */
float ph_read(void)
{
    float voltage = ph_read_voltage();

    if (voltage > 3.2 || voltage < 0.1) {
        ESP_LOGW(TAG, "Sensor pH fuera de rango");
        return -1.0;
    }

    return PH_SLOPE * voltage + PH_OFFSET;
}