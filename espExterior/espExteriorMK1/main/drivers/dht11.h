/**
 * @file dht11.h
 * @brief Interfaz pública del controlador para el sensor DHT11.
 *
 * Este módulo proporciona las definiciones y funciones necesarias para
 * obtener mediciones de temperatura y humedad relativa utilizando el
 * sensor DHT11.
 *
 * El sensor utiliza un protocolo propietario de comunicación basado en
 * temporización precisa de pulsos digitales.
 *
 * @note La implementación del protocolo se encuentra en dht11.c.
 *
 * @authors
 * - Fernando Plazas
 * - Isabella Ordoñez
 * - Juan Daniel Constain
 */

#pragma once

#include <stdint.h>

/**
 * @brief Estructura que almacena las mediciones obtenidas del sensor DHT11.
 *
 * Esta estructura contiene los valores ambientales reportados por el
 * sensor después de una lectura válida.
 */
typedef struct
{
    /**
     * @brief Temperatura ambiente medida.
     *
     * Unidad: grados Celsius (°C).
     */
    float temperatura;

    /**
     * @brief Humedad relativa medida.
     *
     * Unidad: porcentaje (%RH).
     */
    float humedad;

} dht11_data_t;

/**
 * @brief Realiza una lectura completa del sensor DHT11.
 *
 * Esta función ejecuta el protocolo de comunicación requerido por el
 * sensor, recibe los datos transmitidos y verifica su integridad mediante
 * checksum.
 *
 * Si la lectura es exitosa, los valores obtenidos son almacenados en la
 * estructura indicada por el parámetro @p data.
 *
 * @param[out] data Puntero a la estructura donde se almacenarán los datos
 * leídos de temperatura y humedad.
 *
 * @retval 0 Lectura realizada correctamente.
 * @retval -1 Error de comunicación o ausencia de respuesta del sensor.
 * @retval -2 Error de verificación de checksum.
 *
 * @warning Esta función es bloqueante debido a los requerimientos de
 * temporización del protocolo DHT11.
 *
 * @note Se recomienda proteger el acceso mediante mutex si el sensor
 * puede ser utilizado desde múltiples tareas FreeRTOS.
 */
int dht11_read(dht11_data_t *data);