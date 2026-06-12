/**
 * @file ph_sensor.c
 * @brief Implementación del driver para sensor de pH basado en adquisición analógica.
 *
 * Este módulo obtiene mediciones analógicas desde un sensor de pH,
 * calcula un voltaje promedio mediante múltiples muestras y convierte
 * dicho voltaje a unidades de pH utilizando una ecuación de calibración lineal.
 *
 * Funcionalidades:
 * - Lectura de voltaje.
 * - Filtrado mediante promediado.
 * - Conversión voltaje-pH.
 * - Validación básica de rango de operación.
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

/**
 * @brief Canal ADC asociado al sensor de pH.
 *
 * Corresponde al GPIO39 del ESP32.
 */
#define PH_ADC_CHANNEL ADC_CHANNEL_3

/**
 * @brief Cantidad de muestras utilizadas para el cálculo del promedio.
 *
 * El promediado reduce el ruido presente en la señal analógica.
 */
#define NUM_SAMPLES 20

/**
 * @brief Pendiente de la ecuación de calibración.
 *
 * Relaciona el voltaje medido con el valor de pH.
 */
#define PH_SLOPE (-5.7f)

/**
 * @brief Offset de la ecuación de calibración.
 *
 * Corresponde al término independiente obtenido durante
 * el proceso de calibración experimental.
 */
#define PH_OFFSET (21.34f)

/**
 * @brief Voltaje mínimo válido para el sensor.
 */
#define PH_MIN_VOLTAGE (0.1f)

/**
 * @brief Voltaje máximo válido para el sensor.
 */
#define PH_MAX_VOLTAGE (3.2f)

/**
 * @brief Etiqueta utilizada por el sistema de logging.
 */
static const char *TAG = "PH";

// =====================
// INIT
// =====================

/**
 * @brief Inicializa el módulo de pH.
 *
 * Actualmente no requiere configuración específica,
 * ya que la adquisición ADC es gestionada por adc_manager.
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
 * Realiza múltiples lecturas ADC y calcula un promedio
 * para reducir el efecto del ruido presente en la señal.
 *
 * @return Voltaje promedio en voltios.
 */
float ph_read_voltage(void)
{
    float sum = 0;

    for (int i = 0; i < NUM_SAMPLES; i++) {
        sum += adc_manager_read_voltage(PH_ADC_CHANNEL);

        /*
         * Se introduce un pequeño retardo entre muestras
         * para evitar lecturas consecutivas excesivamente
         * correlacionadas y mejorar el promedio obtenido.
         */
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
 * Utiliza una ecuación de calibración lineal:
 *
 * pH = (PH_SLOPE * V) + PH_OFFSET
 *
 * donde:
 * - V corresponde al voltaje medido.
 * - PH_SLOPE es la pendiente de calibración.
 * - PH_OFFSET es el offset de calibración.
 *
 * @return Valor estimado de pH.
 * @retval -1.0 Lectura inválida o fuera de rango.
 */
float ph_read(void)
{
    float voltage = ph_read_voltage();

    if (voltage > PH_MAX_VOLTAGE ||
        voltage < PH_MIN_VOLTAGE)
    {
        ESP_LOGW(TAG, "Sensor pH fuera de rango");
        return -1.0;
    }

    /*
     * Conversión lineal obtenida durante el proceso
     * de calibración del sensor.
     */
    return PH_SLOPE * voltage + PH_OFFSET;
}