/**
 * @file data_types.h
 * @brief Definición de estructuras de datos compartidas del sistema.
 *
 * Este archivo contiene las estructuras utilizadas para el
 * intercambio de información entre las tareas de FreeRTOS
 * mediante colas de comunicación.
 *
 * Define los tipos de datos empleados para:
 * - Gestión de fecha y hora.
 * - Adquisición de sensores.
 * - Comandos de control.
 * - Información para visualización.
 *
 * Estas estructuras permiten desacoplar las tareas del sistema,
 * facilitando una arquitectura modular y escalable.
 *
 * @author
 * Fernando Plazas Trujillo
 * Isabella Ordoñez
 * Juan Daniel Constain
 */

#ifndef DATA_TYPES_H
#define DATA_TYPES_H

#include <stdint.h>
#include <stdbool.h>

// ======================================================
// ESTRUCTURAS COMPARTIDAS ENTRE TAREAS FREERTOS
// ======================================================

/**
 * @brief Estructura para almacenar fecha y hora.
 *
 * Utilizada por el RTC DS3231 y por los módulos
 * de registro de datos y visualización.
 */
typedef struct
{
    uint8_t seconds;   /**< Segundos. */
    uint8_t minutes;   /**< Minutos. */
    uint8_t hours;     /**< Horas. */
    uint8_t day;       /**< Día del mes. */
    uint8_t month;     /**< Mes. */
    uint16_t year;     /**< Año. */

} datetime_t;

/**
 * @brief Datos adquiridos por los sensores del sistema.
 *
 * Esta estructura agrupa todas las variables medidas
 * durante un ciclo de adquisición.
 *
 * Es utilizada para:
 * - Comunicación entre tareas.
 * - Registro en tarjeta SD.
 * - Publicación MQTT.
 * - Visualización en pantalla.
 */
typedef struct
{
    float temperatura; /**< Temperatura interna (°C). */
    float ph;          /**< Valor de pH. */
    float co2;         /**< Concentración estimada de CO2. */
    float co2_raw;     /**< Lectura ADC sin procesar del MQ-135. */

    float voltaje;     /**< Voltaje del sistema. */
    float corriente;   /**< Corriente consumida. */

    datetime_t datetime; /**< Fecha y hora de la medición. */

} sensor_data_t;

/**
 * @brief Comandos generados por la lógica de control.
 *
 * Esta estructura es producida por la tarea de control
 * y consumida por la tarea de actuadores.
 *
 * Permite mantener desacoplada la lógica de decisión
 * del acceso directo al hardware.
 */
typedef struct
{
    bool enfriar;      /**< Activar sistema de enfriamiento. */

    bool bomba;        /**< Activar bomba de circulación. */
    bool mezclar;      /**< Activar motor de mezcla. */

    float servo_angle; /**< Ángulo objetivo del servomotor. */

} control_cmd_t;

/**
 * @brief Información destinada a la interfaz de usuario.
 *
 * Contiene las variables mostradas en la pantalla OLED,
 * incluyendo mediciones de sensores, estados de conexión
 * y fecha/hora actual.
 */
typedef struct
{
    float temperatura; /**< Temperatura actual. */
    float ph;          /**< Valor de pH actual. */
    float co2;         /**< Concentración de CO2 actual. */

    bool wifi_ok;      /**< Estado de conexión WiFi. */
    bool mqtt_ok;      /**< Estado de conexión MQTT. */
    bool sd_ok;        /**< Estado de la tarjeta SD. */

    datetime_t datetime; /**< Fecha y hora mostradas. */

} display_data_t;

#endif /* DATA_TYPES_H */