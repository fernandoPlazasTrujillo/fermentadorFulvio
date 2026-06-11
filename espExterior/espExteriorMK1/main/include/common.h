/**
 * @file common.h
 * @brief Definiciones y estructuras compartidas del sistema.
 *
 * Este archivo contiene tipos de datos comunes utilizados por
 * múltiples módulos del ESP32 externo.
 *
 * @note Las estructuras definidas aquí actúan como interfaz de
 * intercambio de información entre tareas y módulos del sistema.
 *
 * @authors
 * - Fernando Plazas
 * - Isabella Ordoñez
 * - Juan Daniel Constain
 */

#pragma once

#include <stdint.h>

/**
 * @brief Estructura que almacena una medición ambiental completa.
 *
 * Contiene los datos adquiridos por los sensores junto con la
 * marca temporal asociada obtenida desde el RTC DS3231.
 *
 * Esta estructura es utilizada para:
 * - Transferencia de datos entre tareas FreeRTOS.
 * - Almacenamiento en tarjeta SD.
 * - Publicación mediante MQTT.
 * - Visualización en la interfaz de usuario.
 */
typedef struct
{
    /**
     * @brief Temperatura ambiente.
     *
     * Unidad: grados Celsius (°C).
     */
    float temperatura;

    /**
     * @brief Humedad relativa.
     *
     * Unidad: porcentaje (%RH).
     */
    float humedad;

    /**
     * @brief Hora de la medición.
     *
     * Formato de 24 horas.
     */
    uint8_t hora;

    /**
     * @brief Minuto de la medición.
     */
    uint8_t minuto;

    /**
     * @brief Segundo de la medición.
     */
    uint8_t segundo;

    /**
     * @brief Día del mes.
     */
    uint8_t dia;

    /**
     * @brief Mes de la medición.
     */
    uint8_t mes;

    /**
     * @brief Año de la medición.
     */
    uint16_t anio;

} datos_ambiente_t;