/**
 * @file ds3231.h
 * @brief Driver para RTC DS3231.
 *
 * Permite:
 * - Lectura y escritura de fecha/hora
 * - Configuración de alarmas
 *
 * @note Usa comunicación I2C, requiere sincronización mediante mutex.
 *
 * @authors
 * - Fernando Plazas
 * - Isabella Ordoñez
 * - Juan Daniel Constain
 */

#ifndef DS3231_H
#define DS3231_H

#include <stdint.h>

/**
 * @brief Estructura de tiempo del RTC
 */
typedef struct
{
    uint8_t segundos;
    uint8_t minutos;
    uint8_t horas;
    uint8_t dia;
    uint8_t mes;
    uint16_t anio;
} rtc_time_t;

/**
 * @brief Lee la hora actual del RTC
 */
int ds3231_read_time(rtc_time_t *time);

/**
 * @brief Configura la hora del RTC
 */
int ds3231_set_time(rtc_time_t *time);

/**
 * @brief Configura alarma cada minuto
 *
 * Genera interrupción en el pin SQW.
 */
int ds3231_set_alarm_every_minute(void);

/**
 * @brief Limpia el flag de alarma del RTC
 *
 * Necesario para permitir nuevas interrupciones.
 */
int ds3231_clear_alarm_flag(void);

#endif