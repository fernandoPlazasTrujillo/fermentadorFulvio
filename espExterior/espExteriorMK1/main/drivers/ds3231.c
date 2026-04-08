/**
 * @file ds3231.c
 * @brief Implementación del driver RTC DS3231.
 *
 * Maneja comunicación I2C para:
 * - Lectura de tiempo
 * - Configuración de alarmas
 *
 * @note Debe protegerse con mutex si se usa en múltiples tareas.
 *
 * @authors
 * - Fernando Plazas
 * - Isabella Ordoñez
 * - Juan Daniel Constain
 */

#include "ds3231.h"
#include "driver/i2c.h"
#include "esp_log.h"

#define TAG "DS3231"

#define DS3231_ADDR 0x68
#define I2C_PORT I2C_NUM_0

/**
 * @brief Convierte BCD a decimal
 */
static uint8_t bcd_to_dec(uint8_t val)
{
    return ((val >> 4) * 10) + (val & 0x0F);
}

/**
 * @brief Convierte decimal a BCD
 */
static uint8_t dec_to_bcd(uint8_t val)
{
    return ((val / 10) << 4) | (val % 10);
}

// ==========================
// READ TIME
// ==========================
int ds3231_read_time(rtc_time_t *time)
{
    uint8_t data[7];

    i2c_cmd_handle_t cmd = i2c_cmd_link_create();

    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (DS3231_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, 0x00, true);

    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (DS3231_ADDR << 1) | I2C_MASTER_READ, true);

    i2c_master_read(cmd, data, 6, I2C_MASTER_ACK);
    i2c_master_read_byte(cmd, data + 6, I2C_MASTER_NACK);

    i2c_master_stop(cmd);

    esp_err_t ret = i2c_master_cmd_begin(I2C_PORT, cmd, 100 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);

    if (ret != ESP_OK)
        return -1;

    time->segundos = bcd_to_dec(data[0]);
    time->minutos  = bcd_to_dec(data[1]);
    time->horas    = bcd_to_dec(data[2]);
    time->dia      = bcd_to_dec(data[4]);
    time->mes      = bcd_to_dec(data[5]);
    time->anio     = 2000 + bcd_to_dec(data[6]);

    return 0;
}

// ==========================
// SET TIME
// ==========================
int ds3231_set_time(rtc_time_t *time)
{
    uint8_t data[7];

    data[0] = dec_to_bcd(time->segundos);
    data[1] = dec_to_bcd(time->minutos);
    data[2] = dec_to_bcd(time->horas);
    data[3] = 0;
    data[4] = dec_to_bcd(time->dia);
    data[5] = dec_to_bcd(time->mes);
    data[6] = dec_to_bcd(time->anio - 2000);

    i2c_cmd_handle_t cmd = i2c_cmd_link_create();

    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (DS3231_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, 0x00, true);

    for (int i = 0; i < 7; i++)
        i2c_master_write_byte(cmd, data[i], true);

    i2c_master_stop(cmd);

    esp_err_t ret = i2c_master_cmd_begin(I2C_PORT, cmd, 100 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);

    return (ret == ESP_OK) ? 0 : -1;
}

// ==========================
// 🔥 ALARMA CADA MINUTO REAL
// ==========================
int ds3231_set_alarm_every_minute()
{
    esp_err_t ret;

    i2c_cmd_handle_t cmd = i2c_cmd_link_create();

    // Apuntar a registro de alarma 1
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (DS3231_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, 0x07, true);

    // SEGUNDOS = 0 → match exacto
    i2c_master_write_byte(cmd, dec_to_bcd(0), true);

    // MIN, HORA, DIA ignorados (A1M2-A1M4 = 1)
    i2c_master_write_byte(cmd, 0x80, true);
    i2c_master_write_byte(cmd, 0x80, true);
    i2c_master_write_byte(cmd, 0x80, true);

    i2c_master_stop(cmd);

    ret = i2c_master_cmd_begin(I2C_PORT, cmd, 100 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);

    if (ret != ESP_OK)
        return -1;

    // Configurar control: INTCN=1, A1IE=1
    cmd = i2c_cmd_link_create();

    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (DS3231_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, 0x0E, true);

    i2c_master_write_byte(cmd, 0x05, true);

    i2c_master_stop(cmd);

    ret = i2c_master_cmd_begin(I2C_PORT, cmd, 100 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);

    return (ret == ESP_OK) ? 0 : -1;
}

// ==========================
// 🔥 LIMPIAR FLAG (CRÍTICO)
// ==========================
int ds3231_clear_alarm_flag()
{
    uint8_t status;

    i2c_cmd_handle_t cmd = i2c_cmd_link_create();

    // Leer status
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (DS3231_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, 0x0F, true);

    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (DS3231_ADDR << 1) | I2C_MASTER_READ, true);
    i2c_master_read_byte(cmd, &status, I2C_MASTER_NACK);

    i2c_master_stop(cmd);
    i2c_master_cmd_begin(I2C_PORT, cmd, 100 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);

    // Limpiar A1F
    status &= ~0x01;

    cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (DS3231_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, 0x0F, true);
    i2c_master_write_byte(cmd, status, true);
    i2c_master_stop(cmd);

    i2c_master_cmd_begin(I2C_PORT, cmd, 100 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);

    return 0;
}