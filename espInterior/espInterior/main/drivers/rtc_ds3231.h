/**
 * @file rtc_ds3231.h
 * @brief Driver para el reloj en tiempo real DS3231 mediante interfaz I2C.
 *
 * Este módulo proporciona las funciones necesarias para la comunicación
 * con el reloj de tiempo real DS3231.
 *
 * Funcionalidades:
 * - Inicialización del dispositivo.
 * - Lectura de fecha y hora.
 * - Configuración de alarmas.
 * - Gestión de interrupciones mediante alarmas.
 * - Limpieza de flags de alarma.
 *
 * El DS3231 es utilizado como referencia temporal del sistema
 * de fermentación, permitiendo el registro de eventos con timestamp
 * y la activación periódica de tareas mediante interrupciones.
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
 * Verifica la comunicación con el dispositivo y prepara
 * el acceso a los registros internos mediante I2C.
 *
 * Esta función debe ejecutarse durante la fase de
 * inicialización del sistema.
 *
 * @return
 * - ESP_OK: Inicialización exitosa.
 * - Código de error en caso de fallo de comunicación.
 */
esp_err_t ds3231_init(void);

/**
 * @brief Limpia el indicador de alarma del DS3231.
 *
 * Borra el flag de alarma almacenado en el registro
 * de estado del dispositivo, permitiendo la generación
 * de nuevas interrupciones.
 *
 * @return
 * - ESP_OK: Operación realizada correctamente.
 * - Código de error en caso de fallo.
 */
esp_err_t ds3231_clear_alarm_flag(void);

/**
 * @brief Obtiene la fecha y hora actual del RTC.
 *
 * Lee los registros internos del DS3231 y convierte
 * los datos almacenados en formato BCD a formato decimal.
 *
 * @param dt Puntero a la estructura donde se almacenará
 *           la fecha y hora obtenida.
 *
 * @return
 * - ESP_OK: Lectura exitosa.
 * - Código de error en caso de fallo de comunicación.
 */
esp_err_t ds3231_get_datetime(datetime_t *dt);

/**
 * @brief Configura una alarma basada en segundos.
 *
 * Programa la alarma 1 del DS3231 para activarse cuando
 * el valor de segundos coincida con el parámetro indicado.
 *
 * Esta funcionalidad puede utilizarse para generar
 * interrupciones periódicas que despierten al sistema
 * desde modos de bajo consumo.
 *
 * @param seconds Segundo de activación de la alarma.
 *                Rango válido: 0 a 59.
 *
 * @return
 * - ESP_OK: Configuración exitosa.
 * - Código de error en caso contrario.
 */
esp_err_t ds3231_set_alarm_seconds(uint8_t seconds);

#endif