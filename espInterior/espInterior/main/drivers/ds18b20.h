/**
 * @file ds18b20.h
 * @brief Driver para el sensor de temperatura DS18B20 mediante protocolo OneWire.
 *
 * Este módulo proporciona las funciones necesarias para inicializar
 * y adquirir mediciones de temperatura desde un sensor DS18B20.
 *
 * El sensor se utiliza para monitorear la temperatura interna del
 * proceso de fermentación de café.
 *
 * Funcionalidades:
 * - Inicialización del bus OneWire.
 * - Lectura de temperatura en grados Celsius.
 * - Detección básica de errores de comunicación.
 *
 * @author
 * Fernando Plazas Trujillo
 * Isabella Ordoñez
 * Juan Daniel Constain
 */

#ifndef DS18B20_H
#define DS18B20_H

#include "esp_err.h"
#include "driver/gpio.h"

/**
 * @brief Inicializa el sensor DS18B20.
 *
 * Configura el GPIO utilizado para la comunicación OneWire y
 * habilita la resistencia pull-up requerida por el protocolo.
 *
 * Esta función debe ejecutarse una única vez durante la
 * inicialización del sistema.
 *
 * @param pin GPIO al que se encuentra conectado el sensor.
 *
 * @return
 * - ESP_OK: Inicialización exitosa.
 * - ESP_FAIL: Error durante la configuración del dispositivo.
 */
esp_err_t ds18b20_init(gpio_num_t pin);

/**
 * @brief Obtiene una medición de temperatura desde el DS18B20.
 *
 * Ejecuta la secuencia completa de comunicación OneWire:
 * - Reset del bus.
 * - Inicio de conversión de temperatura.
 * - Lectura de memoria Scratchpad.
 * - Conversión del dato a grados Celsius.
 *
 * @return Temperatura medida en grados Celsius.
 * @retval -127.0 Error de comunicación o ausencia del sensor.
 */
float ds18b20_read_temperature(void);

#endif