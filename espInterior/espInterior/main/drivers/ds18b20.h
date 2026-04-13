/**
 * @file ds18b20.h
 * @brief Driver para el sensor de temperatura DS18B20 usando protocolo OneWire.
 * 
 * Permite inicializar el sensor y leer la temperatura en grados Celsius.
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
 * Configura el pin GPIO y activa la resistencia pull-up necesaria
 * para la comunicación OneWire.
 * 
 * @param pin GPIO donde está conectado el sensor.
 * @return ESP_OK si la inicialización fue exitosa.
 */
esp_err_t ds18b20_init(gpio_num_t pin);

/**
 * @brief Lee la temperatura desde el sensor DS18B20.
 * 
 * Realiza la secuencia completa:
 * - Reset del bus OneWire
 * - Conversión de temperatura
 * - Lectura de datos
 * 
 * @return Temperatura en grados Celsius.
 * @return -127.0 si el sensor no responde.
 */
float ds18b20_read_temperature(void);

#endif