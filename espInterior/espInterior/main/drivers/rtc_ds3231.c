/**
 * @file rtc_ds3231.c
 * @brief Implementación del driver para el RTC DS3231.
 * 
 * Maneja la comunicación I2C para lectura de fecha/hora,
 * configuración de alarmas y control del dispositivo.
 * 
 * Incluye conversión entre formato BCD y decimal.
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

#define DS3231_ADDR 0x68

extern SemaphoreHandle_t mutex_i2c;

static const char *TAG = "DS3231";

// =====================
// INIT
// =====================

/**
 * @brief Inicializa el RTC DS3231.
 * 
 * @return ESP_OK siempre que se registre correctamente.
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
 * @brief Convierte un valor en formato BCD a decimal.
 * 
 * @param val Valor en BCD.
 * @return Valor en decimal.
 */
static uint8_t bcd_to_dec(uint8_t val)
{
    return (val >> 4) * 10 + (val & 0x0F);
}

/**
 * @brief Convierte un valor decimal a formato BCD.
 * 
 * @param val Valor decimal.
 * @return Valor en BCD.
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
 * @param dt Estructura donde se almacenan los datos.
 * @return ESP_OK si la operación se ejecuta.
 */
esp_err_t ds3231_get_datetime(datetime_t *dt)
{
    uint8_t data[7];

    if (xSemaphoreTake(mutex_i2c, pdMS_TO_TICKS(100)) == pdTRUE) {

        i2c_manager_read(DS3231_ADDR, 0x00, data, 7);
        xSemaphoreGive(mutex_i2c);

        dt->seconds = bcd_to_dec(data[0]);
        dt->minutes = bcd_to_dec(data[1]);
        dt->hours   = bcd_to_dec(data[2]);
        dt->day     = bcd_to_dec(data[4]);
        dt->month   = bcd_to_dec(data[5] & 0x1F);
        dt->year    = 2000 + bcd_to_dec(data[6]);
    }

    return ESP_OK;
}

// =====================
// CLEAR FLAG
// =====================

/**
 * @brief Limpia el flag de alarma del DS3231.
 * 
 * @return ESP_OK si la operación se ejecuta.
 */
esp_err_t ds3231_clear_alarm_flag(void)
{
    uint8_t status;

    if (xSemaphoreTake(mutex_i2c, pdMS_TO_TICKS(100)) == pdTRUE) {

        i2c_manager_read(DS3231_ADDR, 0x0F, &status, 1);
        status &= ~0x01;
        i2c_manager_write(DS3231_ADDR, 0x0F, &status, 1);

        xSemaphoreGive(mutex_i2c);
    }

    return ESP_OK;
}

// =====================
// SET ALARM
// =====================

/**
 * @brief Configura una alarma por coincidencia de segundos.
 * 
 * @param seconds Segundo en el que se activa la alarma.
 * @return ESP_OK si la configuración fue exitosa.
 */
esp_err_t ds3231_set_alarm_seconds(uint8_t seconds)
{
    uint8_t data[4];

    data[0] = dec_to_bcd(seconds);
    data[1] = 0x80;
    data[2] = 0x80;
    data[3] = 0x80;

    if (xSemaphoreTake(mutex_i2c, pdMS_TO_TICKS(100)) == pdTRUE) {

        i2c_manager_write(DS3231_ADDR, 0x07, data, 4);

        uint8_t control = 0x05;
        i2c_manager_write(DS3231_ADDR, 0x0E, &control, 1);

        xSemaphoreGive(mutex_i2c);
    }

    ESP_LOGI(TAG, "Alarma configurada en segundo: %d", seconds);

    return ESP_OK;
}