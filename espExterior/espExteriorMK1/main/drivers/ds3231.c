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

 /**
 * @section rtc_architecture Arquitectura del sistema
 *
 * El módulo DS3231 constituye la referencia temporal principal
 * del sistema distribuido de fermentación de café.
 *
 * Sus funciones principales son:
 * - Generación de timestamps para almacenamiento en SD.
 * - Sincronización de mediciones ambientales.
 * - Activación periódica del sistema mediante alarmas.
 * - Soporte para estrategias de ahorro energético basadas en sleep.
 */

#include "ds3231.h"
#include "driver/i2c.h"
#include "esp_log.h"

/**
 * @brief Etiqueta utilizada para mensajes de depuración.
 */
#define TAG "DS3231"

/**
 * @brief Dirección I2C del RTC DS3231.
 */
#define DS3231_ADDR 0x68

/**
 * @brief Puerto I2C utilizado para la comunicación.
 */
#define I2C_PORT I2C_NUM_0

/**
 * @brief Convierte un valor codificado en BCD a decimal.
 *
 * El DS3231 almacena los valores de fecha y hora en formato
 * Binary Coded Decimal (BCD), por lo que es necesario convertirlos
 * antes de utilizarlos en la aplicación.
 *
 * @param val Valor en formato BCD.
 * @return Valor convertido a decimal.
 */
static uint8_t bcd_to_dec(uint8_t val)
{
    return ((val >> 4) * 10) + (val & 0x0F);
}

/**
 * @brief Convierte un valor decimal a formato BCD.
 *
 * Esta función se utiliza antes de escribir valores de fecha
 * y hora en los registros internos del DS3231.
 *
 * @param val Valor decimal.
 * @return Valor convertido a BCD.
 */
static uint8_t dec_to_bcd(uint8_t val)
{
    return ((val / 10) << 4) | (val % 10);
}

/**
 * @brief Obtiene la fecha y hora actuales del DS3231.
 *
 * Lee los registros de tiempo del RTC mediante comunicación I2C
 * y convierte los valores almacenados en formato BCD a decimal.
 *
 * Los campos recuperados son:
 * - Segundos
 * - Minutos
 * - Horas
 * - Día del mes
 * - Mes
 * - Año
 *
 * @param[out] time Estructura donde se almacenará la fecha y hora.
 *
 * @retval 0 Lectura exitosa.
 * @retval -1 Error de comunicación I2C.
 *
 * @note Esta función debe ejecutarse bajo protección de mutex
 * si el bus I2C es compartido con otros dispositivos.
 */
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

/**
 * @brief Configura la fecha y hora del RTC DS3231.
 *
 * Convierte los valores de la estructura rtc_time_t a formato BCD
 * y los almacena en los registros internos del RTC.
 *
 * @param[in] time Estructura que contiene la fecha y hora a configurar.
 *
 * @retval 0 Configuración exitosa.
 * @retval -1 Error de comunicación I2C.
 *
 * @warning Esta operación modifica la referencia temporal utilizada
 * por todo el sistema para generación de timestamps y alarmas.
 */
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

/**
 * @brief Configura una alarma periódica cada minuto.
 *
 * Programa la alarma 1 del DS3231 para generar una interrupción
 * cuando los segundos alcancen el valor 0.
 *
 * La configuración utilizada permite generar una interrupción
 * exactamente al inicio de cada minuto.
 *
 * Registros utilizados:
 * - Alarm1 (0x07 - 0x0A)
 * - Control (0x0E)
 *
 * Esta funcionalidad se utiliza para despertar periódicamente
 * al ESP32 desde modos de bajo consumo.
 *
 * @retval 0 Configuración exitosa.
 * @retval -1 Error de comunicación I2C.
 *
 * @note El pin INT/SQW del DS3231 debe estar conectado a un GPIO
 * configurado como interrupción externa.
 */
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

/**
 * @brief Limpia el indicador de alarma A1F.
 *
 * Después de que una alarma genera una interrupción, el bit A1F
 * permanece activado dentro del registro de estado del DS3231.
 *
 * Esta función limpia dicho bit para permitir que la siguiente
 * alarma vuelva a producir una interrupción válida.
 *
 * Registro utilizado:
 * - Status Register (0x0F)
 *
 * @retval 0 Operación completada.
 *
 * @warning Si esta función no se ejecuta después de atender la
 * interrupción, las alarmas posteriores no funcionarán correctamente.
 */
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