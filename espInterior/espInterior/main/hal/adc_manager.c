/**
 * @file adc_manager.c
 * @brief Implementación del manejo del ADC del ESP32.
 * 
 * Utiliza el modo one-shot con calibración para obtener lecturas
 * precisas de voltaje en distintos canales.
 * 
 * Soporta múltiples sensores analógicos del sistema.
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

static const char *TAG = "ADC_MANAGER";

// =====================
// HANDLES
// =====================

/** @brief Handle del ADC en modo one-shot */
static adc_oneshot_unit_handle_t adc1_handle;

/** @brief Handle de calibración del ADC */
static adc_cali_handle_t adc1_cali_handle;

// =====================
// INIT
// =====================

/**
 * @brief Inicializa el ADC y su sistema de calibración.
 */
void adc_manager_init(void)
{
    adc_oneshot_unit_init_cfg_t init_config = {
        .unit_id = ADC_UNIT_1,
    };

    adc_oneshot_new_unit(&init_config, &adc1_handle);

    adc_oneshot_chan_cfg_t config = {
        .bitwidth = ADC_BITWIDTH_DEFAULT,
        .atten = ADC_ATTEN_DB_12,
    };

    adc_oneshot_config_channel(adc1_handle, ADC_CHANNEL_0, &config);
    adc_oneshot_config_channel(adc1_handle, ADC_CHANNEL_3, &config);
    adc_oneshot_config_channel(adc1_handle, ADC_CHANNEL_4, &config);

    adc_cali_line_fitting_config_t cali_config = {
        .unit_id = ADC_UNIT_1,
        .atten = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };

    adc_cali_create_scheme_line_fitting(&cali_config, &adc1_cali_handle);

    ESP_LOGI(TAG, "ADC1 inicializado y calibrado");
}

// =====================
// READ
// =====================

/**
 * @brief Lee el voltaje de un canal ADC.
 * 
 * @param channel Canal ADC a leer.
 * @return Voltaje en voltios.
 */
float adc_manager_read_voltage(adc_channel_t channel)
{
    int raw;
    int voltage_mv;

    adc_oneshot_read(adc1_handle, channel, &raw);
    adc_cali_raw_to_voltage(adc1_cali_handle, raw, &voltage_mv);

    return voltage_mv / 1000.0;
}