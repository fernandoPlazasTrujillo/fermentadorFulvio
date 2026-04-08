/**
 * @file dht11.h
 * @brief Driver para sensor de temperatura y humedad DHT11.
 *
 * Permite la lectura de datos ambientales mediante protocolo propietario
 * basado en temporización precisa.
 *
 * @authors
 * - Fernando Plazas
 * - Isabella Ordoñez
 * - Juan Daniel Constain
 */

#pragma once

#include <stdint.h>

/**
 * @brief Estructura de datos del sensor DHT11
 */
typedef struct {
    float temperatura; /**< Temperatura en grados Celsius */
    float humedad;     /**< Humedad relativa en porcentaje */
} dht11_data_t;

/**
 * @brief Realiza una lectura del sensor DHT11
 *
 * @param data Puntero a estructura donde se almacenarán los datos
 * @return int
 * - 0: lectura exitosa
 * - -1: error de comunicación
 * - -2: error de checksum
 */
int dht11_read(dht11_data_t *data);