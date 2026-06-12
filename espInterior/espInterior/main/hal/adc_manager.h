/**
 * @file adc_manager.h
 * @brief Capa de abstracción para el convertidor analógico-digital (ADC) del ESP32.
 *
 * Este módulo implementa una interfaz de acceso al ADC utilizando
 * el modo One-Shot proporcionado por ESP-IDF.
 *
 * Funcionalidades:
 * - Inicialización del periférico ADC.
 * - Configuración de canales analógicos.
 * - Calibración de lecturas.
 * - Conversión de valores ADC a voltaje.
 *
 * Esta capa permite desacoplar los drivers de sensores
 * del hardware específico del ESP32, facilitando la
 * reutilización y mantenimiento del software.
 *
 * Los módulos:
 * - mq135
 * - ph_sensor
 * - voltage_sensor
 *
 * utilizan este HAL para adquirir señales analógicas.
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
 * @brief Inicializa el subsistema ADC.
 *
 * Configura la unidad ADC en modo One-Shot y prepara
 * los mecanismos de calibración utilizados para mejorar
 * la precisión de las conversiones.
 *
 * Esta función debe ejecutarse una única vez durante
 * la inicialización del sistema.
 */
void adc_manager_init(void);

/**
 * @brief Obtiene el voltaje asociado a un canal ADC.
 *
 * El procedimiento incluye:
 * - Conversión analógica-digital.
 * - Calibración de la lectura.
 * - Conversión de milivoltios a voltios.
 *
 * @param channel Canal ADC a convertir.
 *
 * @return Voltaje medido en voltios.
 */
float adc_manager_read_voltage(adc_channel_t channel);

#endif