/**
 * @file rtc_ds3231.c
 * @brief Implementación del driver para el RTC DS3231.
 *
 * Maneja la comunicación I2C para lectura de fecha/hora,
 * configuración de alarmas y control del dispositivo.
 *
 * Funcionalidades:
 * - Lectura de fecha y hora.
 * - Configuración de alarmas.
 * - Limpieza de flags de alarma.
 * - Conversión entre formato BCD y decimal.
 *
 * El RTC DS3231 actúa como referencia temporal del sistema
 * de fermentación y permite la generación de eventos periódicos
 * mediante interrupciones configuradas por alarma.
 *
 * @author
 * Fernando Plazas Trujillo
 * Isabella Ordoñez
 * Juan Daniel Constain
 */

#include "rtc_ds3231.h"
#include "hal/i2c_manager.h"

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

#include "esp_log.h"

/**
 * @brief Dirección I2C del RTC DS3231.
 */
#define DS3231_ADDR 0x68

/**
 * @brief Valor de configuración del registro de control.
 *
 * INTCN = 1  -> Modo interrupción.
 * A1IE  = 1  -> Habilita interrupción de Alarm 1.
 */
#define DS3231_CONTROL_A1_INTERRUPT 0x05

/**
 * @brief Mutex compartido para acceso exclusivo al bus I2C.
 *
 * Garantiza acceso seguro al RTC cuando múltiples tareas
 * utilizan periféricos conectados al mismo bus.
 */
extern SemaphoreHandle_t mutex_i2c;

/**
 * @brief Etiqueta utilizada por el sistema de logging.
 */
static const char *TAG = "DS3231";

// =====================
// INIT
// =====================

/**
 * @brief Inicializa el módulo RTC DS3231.
 *
 * Actualmente no requiere configuración específica,
 * ya que el acceso al dispositivo se realiza bajo demanda
 * mediante transacciones I2C.
 *
 * @return ESP_OK.
 */
esp_err_t ds3231_init(void)
{
    ESP_LOGI(TAG, "DS3231 inicializado");
    return ESP_OK;
}

// =====================
// BCD ↔ DEC
// =====================

/**
 * @brief Convierte un valor codificado en BCD a decimal.
 *
 * El DS3231 almacena los valores de fecha y hora en formato
 * Binary Coded Decimal (BCD), por lo que es necesario
 * convertirlos antes de utilizarlos en la aplicación.
 *
 * @param val Valor en formato BCD.
 *
 * @return Valor equivalente en decimal.
 */
static uint8_t bcd_to_dec(uint8_t val)
{
    return (val >> 4) * 10 + (val & 0x0F);
}

/**
 * @brief Convierte un valor decimal a formato BCD.
 *
 * Utilizado para escribir configuraciones de tiempo
 * y alarmas en los registros internos del DS3231.
 *
 * @param val Valor decimal.
 *
 * @return Valor equivalente en formato BCD.
 */
static uint8_t dec_to_bcd(uint8_t val)
{
    return ((val / 10) << 4) | (val % 10);
}

// =====================
// GET DATETIME
// =====================

/**
 * @brief Lee la fecha y hora actual desde el DS3231.
 *
 * Lee los registros internos del dispositivo y convierte
 * los valores almacenados en formato BCD a formato decimal.
 *
 * @param dt Puntero a la estructura donde se almacenará
 *           la fecha y hora obtenida.
 *
 * @return
 * - ESP_OK: Lectura exitosa.
 * - ESP_ERR_TIMEOUT: No fue posible obtener el mutex I2C.
 */
esp_err_t ds3231_get_datetime(datetime_t *dt)
{
    uint8_t data[7];

    if (xSemaphoreTake(mutex_i2c, pdMS_TO_TICKS(100)) != pdTRUE) {
        return ESP_ERR_TIMEOUT;
    }

    i2c_manager_read(DS3231_ADDR, 0x00, data, 7);

    xSemaphoreGive(mutex_i2c);

    /*
     * Conversión de registros DS3231 almacenados
     * en formato BCD hacia formato decimal.
     */
    dt->seconds = bcd_to_dec(data[0]);
    dt->minutes = bcd_to_dec(data[1]);
    dt->hours   = bcd_to_dec(data[2]);
    dt->day     = bcd_to_dec(data[4]);
    dt->month   = bcd_to_dec(data[5] & 0x1F);
    dt->year    = 2000 + bcd_to_dec(data[6]);

    return ESP_OK;
}

// =====================
// CLEAR FLAG
// =====================

/**
 * @brief Limpia el flag de alarma del DS3231.
 *
 * Borra el bit A1F del registro de estado para permitir
 * futuras interrupciones de alarma.
 *
 * @return
 * - ESP_OK: Operación exitosa.
 * - ESP_ERR_TIMEOUT: No fue posible obtener el mutex I2C.
 */
esp_err_t ds3231_clear_alarm_flag(void)
{
    uint8_t status;

    if (xSemaphoreTake(mutex_i2c, pdMS_TO_TICKS(100)) != pdTRUE) {
        return ESP_ERR_TIMEOUT;
    }

    i2c_manager_read(DS3231_ADDR, 0x0F, &status, 1);

    status &= ~0x01;

    i2c_manager_write(DS3231_ADDR, 0x0F, &status, 1);

    xSemaphoreGive(mutex_i2c);

    return ESP_OK;
}

// =====================
// SET ALARM
// =====================

/**
 * @brief Configura una alarma basada en coincidencia de segundos.
 *
 * Programa la Alarm 1 del DS3231 para generar una interrupción
 * cuando el campo de segundos coincida con el valor indicado.
 *
 * @param seconds Segundo objetivo de activación (0-59).
 *
 * @return
 * - ESP_OK: Configuración exitosa.
 * - ESP_ERR_TIMEOUT: No fue posible obtener el mutex I2C.
 */
esp_err_t ds3231_set_alarm_seconds(uint8_t seconds)
{
    uint8_t data[4];

    /*
     * Configuración de Alarm 1:
     * - Coincidencia únicamente sobre segundos.
     * - Minutos, horas y día ignorados mediante bits A1Mx.
     */
    data[0] = dec_to_bcd(seconds);
    data[1] = 0x80;
    data[2] = 0x80;
    data[3] = 0x80;

    if (xSemaphoreTake(mutex_i2c, pdMS_TO_TICKS(100)) != pdTRUE) {
        return ESP_ERR_TIMEOUT;
    }

    i2c_manager_write(DS3231_ADDR, 0x07, data, 4);

    /*
     * INTCN = 1  -> modo interrupción
     * A1IE  = 1  -> habilita Alarm 1
     */
    uint8_t control = DS3231_CONTROL_A1_INTERRUPT;

    i2c_manager_write(DS3231_ADDR, 0x0E, &control, 1);

    xSemaphoreGive(mutex_i2c);

    ESP_LOGI(TAG, "Alarma configurada en segundo: %d", seconds);

    return ESP_OK;
}