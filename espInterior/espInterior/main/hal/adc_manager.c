/**
 * @file adc_manager.c
 * @brief Implementación de la capa de abstracción para el ADC del ESP32.
 *
 * Este módulo implementa el acceso al convertidor analógico-digital
 * utilizando el modo One-Shot proporcionado por ESP-IDF.
 *
 * Funcionalidades:
 * - Inicialización del ADC1.
 * - Configuración de canales analógicos.
 * - Calibración mediante Line Fitting.
 * - Conversión de valores ADC a voltaje.
 *
 * Los sensores analógicos del sistema utilizan este módulo
 * como interfaz única de acceso al ADC.
 *
 * Sensores asociados:
 * - MQ135
 * - Sensor de pH
 * - Sensor de voltaje
 *
 * @author
 * Fernando Plazas Trujillo
 * Isabella Ordoñez
 * Juan Daniel Constain
 */

#include "adc_manager.h"

#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#include "esp_log.h"

/**
 * @brief Etiqueta utilizada por el sistema de logging.
 */
static const char *TAG = "ADC_MANAGER";

// =====================
// HANDLES
// =====================

/**
 * @brief Handle de la unidad ADC1 en modo One-Shot.
 */
static adc_oneshot_unit_handle_t adc1_handle;

/**
 * @brief Handle de calibración ADC.
 *
 * Utilizado para convertir lecturas crudas a voltajes
 * compensados mediante el esquema de calibración.
 */
static adc_cali_handle_t adc1_cali_handle;

// =====================
// INIT
// =====================

/**
 * @brief Inicializa el ADC y el sistema de calibración.
 *
 * Configura:
 * - ADC Unit 1.
 * - Resolución por defecto.
 * - Atenuación de 12 dB.
 * - Esquema de calibración Line Fitting.
 *
 * Además registra todos los canales utilizados por
 * los sensores analógicos del sistema.
 */
void adc_manager_init(void)
{
    /**
     * Configuración de la unidad ADC1.
     */
    adc_oneshot_unit_init_cfg_t init_config = {
        .unit_id = ADC_UNIT_1,
    };

    adc_oneshot_new_unit(
        &init_config,
        &adc1_handle);

    /**
     * Configuración común para todos los canales.
     *
     * Atenuación de 12 dB para ampliar el rango de medida.
     */
    adc_oneshot_chan_cfg_t config = {
        .bitwidth = ADC_BITWIDTH_DEFAULT,
        .atten = ADC_ATTEN_DB_12,
    };

    /*
     * ADC_CHANNEL_0 -> MQ135
     * ADC_CHANNEL_3 -> Sensor pH
     * ADC_CHANNEL_4 -> Sensor voltaje
     */
    adc_oneshot_config_channel(
        adc1_handle,
        ADC_CHANNEL_0,
        &config);

    adc_oneshot_config_channel(
        adc1_handle,
        ADC_CHANNEL_3,
        &config);

    adc_oneshot_config_channel(
        adc1_handle,
        ADC_CHANNEL_4,
        &config);

    /**
     * Configuración de calibración ADC.
     *
     * El esquema Line Fitting mejora la precisión de
     * las conversiones compensando errores internos.
     */
    adc_cali_line_fitting_config_t cali_config = {
        .unit_id = ADC_UNIT_1,
        .atten = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };

    adc_cali_create_scheme_line_fitting(
        &cali_config,
        &adc1_cali_handle);

    ESP_LOGI(TAG, "ADC1 inicializado y calibrado");
}

// =====================
// READ
// =====================

/**
 * @brief Obtiene el voltaje asociado a un canal ADC.
 *
 * El procedimiento consiste en:
 * - Leer el valor ADC crudo.
 * - Aplicar calibración.
 * - Convertir milivoltios a voltios.
 *
 * @param channel Canal ADC a convertir.
 *
 * @return Voltaje medido en voltios.
 */
float adc_manager_read_voltage(adc_channel_t channel)
{
    int raw;
    int voltage_mv;

    /*
     * Lectura ADC sin procesar.
     */
    adc_oneshot_read(
        adc1_handle,
        channel,
        &raw);

    /*
     * Conversión de lectura cruda a milivoltios
     * mediante calibración.
     */
    adc_cali_raw_to_voltage(
        adc1_cali_handle,
        raw,
        &voltage_mv);

    /*
     * Conversión de mV a V.
     */
    return voltage_mv / 1000.0f;
}