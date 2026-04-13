/*
 * @file adc_manager.h
 * @brief Abstracción del ADC del ESP32 usando modo one-shot.
 * 
 * Permite inicializar el ADC y obtener lecturas de voltaje
 * calibradas en diferentes canales.
 * 
 * @author
 * Fernando Plazas Trujillo
 * Isabella Ordoñez
 * Juan Daniel Constain
 */

#ifndef ADC_MANAGER_H
#define ADC_MANAGER_H

#include "esp_adc/adc_oneshot.h"

/**
 * @brief Inicializa el ADC en modo one-shot.
 * 
 * Configura los canales y aplica calibración para mejorar
 * la precisión de las lecturas.
 */
void adc_manager_init(void);

/**
 * @brief Lee el voltaje de un canal ADC.
 * 
 * Realiza:
 * - Lectura cruda
 * - Conversión a milivoltios
 * - Conversión a voltios
 * 
 * @param channel Canal ADC a leer.
 * @return Voltaje en voltios.
 */
float adc_manager_read_voltage(adc_channel_t channel);

#endif