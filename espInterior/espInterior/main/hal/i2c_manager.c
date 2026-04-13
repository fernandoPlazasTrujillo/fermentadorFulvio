/**
 * @file i2c_manager.c
 * @brief Implementación del manejo del bus I2C.
 * 
 * Este módulo configura el ESP32 como maestro I2C y permite
 * la comunicación con dispositivos mediante lectura y escritura.
 * 
 * Incluye protección mediante mutex para acceso concurrente.
 * 
 * @author
 * Fernando Plazas Trujillo
 * Isabella Ordoñez
 * Juan Daniel Constain
 */

#include "i2c_manager.h"
#include "driver/i2c.h"
#include "esp_log.h"

#define TAG "I2C"

/** @brief Pin SCL */
#define I2C_MASTER_SCL_IO 22

/** @brief Pin SDA */
#define I2C_MASTER_SDA_IO 21

/** @brief Puerto I2C */
#define I2C_MASTER_NUM I2C_NUM_0

/** @brief Frecuencia del bus I2C */
#define I2C_MASTER_FREQ_HZ 100000

/** @brief Mutex para acceso al bus I2C */
SemaphoreHandle_t mutex_i2c = NULL;

// =====================
// INIT
// =====================

/**
 * @brief Inicializa el bus I2C en modo maestro.
 */
void i2c_manager_init(void)
{
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ
    };

    i2c_param_config(I2C_MASTER_NUM, &conf);
    i2c_driver_install(I2C_MASTER_NUM, conf.mode, 0, 0, 0);

    mutex_i2c = xSemaphoreCreateMutex();

    ESP_LOGI(TAG, "I2C inicializado");
}

// =====================
// WRITE
// =====================

/**
 * @brief Escribe datos en un dispositivo I2C.
 * 
 * @param addr Dirección del dispositivo.
 * @param reg Registro destino.
 * @param data Datos a escribir.
 * @param len Cantidad de bytes.
 * @return Código de error ESP.
 */
esp_err_t i2c_manager_write(uint8_t addr, uint8_t reg, uint8_t *data, size_t len)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();

    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (addr << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, reg, true);
    i2c_master_write(cmd, data, len, true);
    i2c_master_stop(cmd);

    esp_err_t ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, pdMS_TO_TICKS(100));

    i2c_cmd_link_delete(cmd);

    return ret;
}

// =====================
// READ
// =====================

/**
 * @brief Lee datos desde un dispositivo I2C.
 * 
 * @param addr Dirección del dispositivo.
 * @param reg Registro origen.
 * @param data Buffer de lectura.
 * @param len Número de bytes a leer.
 * @return Código de error ESP.
 */
esp_err_t i2c_manager_read(uint8_t addr, uint8_t reg, uint8_t *data, size_t len)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();

    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (addr << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, reg, true);

    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (addr << 1) | I2C_MASTER_READ, true);

    if (len > 1) {
        i2c_master_read(cmd, data, len - 1, I2C_MASTER_ACK);
    }

    i2c_master_read_byte(cmd, data + len - 1, I2C_MASTER_NACK);
    i2c_master_stop(cmd);

    esp_err_t ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, pdMS_TO_TICKS(100));

    i2c_cmd_link_delete(cmd);

    return ret;
}