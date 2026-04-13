/**
 * @file rtc_ds3231.h
 * @brief Driver para el reloj en tiempo real DS3231 mediante I2C.
 * 
 * Permite inicializar el RTC, obtener fecha/hora actual,
 * limpiar flags de alarma y configurar alarmas.
 * 
 * @author
 * Fernando Plazas Trujillo
 * Isabella Ordoñez
 * Juan Daniel Constain
 */

#ifndef RTC_DS3231_H
#define RTC_DS3231_H

#include "esp_err.h"
#include "utils/data_types.h"

/**
 * @brief Inicializa el módulo RTC DS3231.
 * 
 * @return ESP_OK si la inicialización fue exitosa.
 */
esp_err_t ds3231_init(void);

/**
 * @brief Limpia el flag de alarma del DS3231.
 * 
 * @return ESP_OK si la operación fue exitosa.
 */
esp_err_t ds3231_clear_alarm_flag(void);

/**
 * @brief Obtiene la fecha y hora actual del RTC.
 * 
 * @param dt Puntero a estructura donde se almacenará la fecha/hora.
 * @return ESP_OK si la lectura fue exitosa.
 */
esp_err_t ds3231_get_datetime(datetime_t *dt);

/**
 * @brief Configura una alarma basada en segundos.
 * 
 * La alarma se activa cuando el valor de segundos coincide.
 * 
 * @param seconds Segundo en el que se activará la alarma (0–59).
 * @return ESP_OK si la configuración fue exitosa.
 */
esp_err_t ds3231_set_alarm_seconds(uint8_t seconds);

#endif