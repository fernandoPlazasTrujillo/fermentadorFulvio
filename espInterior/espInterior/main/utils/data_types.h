/*
 * @file data_types.h
 * @brief Definición de estructuras de datos del sistema.
 * 
 * Contiene las estructuras utilizadas para:
 * - Manejo de fecha y hora
 * - Datos de sensores
 * - Comandos de control
 * 
 * Estas estructuras son compartidas entre tasks mediante colas.
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

// =====================
// DATETIME
// =====================

/**
 * @brief Estructura de fecha y hora.
 */
typedef struct {
    uint8_t seconds;
    uint8_t minutes;
    uint8_t hours;
    uint8_t day;
    uint8_t month;
    uint16_t year;
} datetime_t;

// =====================
// DATOS DE SENSORES
// =====================

/**
 * @brief Datos provenientes de los sensores del sistema.
 */
typedef struct {
    float temperatura;
    float ph;
    float co2;
    float co2_raw;

    float voltaje;
    float corriente;

    datetime_t datetime;
} sensor_data_t;

// =====================
// COMANDOS DE CONTROL
// =====================

/**
 * @brief Comandos enviados a los actuadores.
 */
typedef struct {

    bool enfriar;

    bool bomba;
    bool mezclar;

    float servo_angle;

} control_cmd_t;

#endif

