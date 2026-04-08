#ifndef DATA_TYPES_H
#define DATA_TYPES_H

#include <stdint.h>
#include <stdbool.h>   // 🔥 FALTABA ESTO

// =====================
// DATETIME
// =====================
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
typedef struct {

    // 🔥 mantener compatibilidad con task_control
    bool enfriar;

    // actuadores reales
    bool bomba;
    bool mezclar;

    // servo
    float servo_angle;

} control_cmd_t;

#endif