/**
 * @file i2c_manager.c
 * @brief Implementación de la capa de abstracción para el bus I2C del ESP32.
 *
 * Este módulo configura el ESP32 como maestro I2C y proporciona
 * funciones genéricas para comunicación con dispositivos conectados
 * al bus.
 *
 * Funcionalidades:
 * - Inicialización del bus I2C.
 * - Escritura de registros.
 * - Lectura de registros.
 * - Escritura de bloques de datos.
 * - Protección mediante mutex para acceso concurrente.
 *
 * Los módulos que utilizan esta capa incluyen:
 * - rtc_ds3231
 * - oled_display
 *
 * @author
 * Fernando Plazas Trujillo
 * Isabella Ordoñez
 * Juan Daniel Constain
 */

#include "i2c_manager.h"

#include "driver/i2c.h"
#include "esp_log.h"

/**
 * @brief Etiqueta utilizada por el sistema de logging.
 */
#define TAG "I2C"

/**
 * @brief GPIO utilizado como línea SCL.
 */
#define I2C_MASTER_SCL_IO 22

/**
 * @brief GPIO utilizado como línea SDA.
 */
#define I2C_MASTER_SDA_IO 21

/**
 * @brief Puerto I2C utilizado por el sistema.
 */
#define I2C_MASTER_NUM I2C_NUM_0

/**
 * @brief Frecuencia de operación del bus I2C.
 *
 * Configurada a 100 kHz para garantizar compatibilidad
 * con todos los dispositivos conectados.
 */
#define I2C_MASTER_FREQ_HZ 100000

/**
 * @brief Mutex global para acceso seguro al bus I2C.
 *
 * Garantiza exclusión mutua cuando múltiples tareas
 * intentan acceder simultáneamente a dispositivos
 * conectados al mismo bus.
 */
SemaphoreHandle_t mutex_i2c = NULL;

// =====================
// INIT
// =====================

/**
 * @brief Inicializa el bus I2C en modo maestro.
 *
 * Configura:
 * - Pines SDA y SCL.
 * - Pull-ups internas.
 * - Frecuencia de operación.
 * - Driver I2C de ESP-IDF.
 *
 * Además crea el mutex utilizado para sincronizar
 * el acceso concurrente al bus.
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

    /*
     * Configuración del periférico I2C.
     */
    i2c_param_config(
        I2C_MASTER_NUM,
        &conf);

    /*
     * Instalación del driver I2C.
     */
    i2c_driver_install(
        I2C_MASTER_NUM,
        conf.mode,
        0,
        0,
        0);

    /*
     * Creación del mutex global para protección
     * de acceso al bus compartido.
     */
    mutex_i2c = xSemaphoreCreateMutex();

    ESP_LOGI(TAG, "I2C inicializado");
}

// =====================
// WRITE
// =====================

/**
 * @brief Escribe datos en un registro de un dispositivo I2C.
 *
 * La secuencia implementada es:
 *
 * START
 * -> Dirección dispositivo + Write
 * -> Registro destino
 * -> Datos
 * -> STOP
 *
 * @param addr Dirección I2C del dispositivo.
 * @param reg Registro de destino.
 * @param data Datos a transmitir.
 * @param len Número de bytes a enviar.
 *
 * @return Código de error ESP-IDF.
 */
esp_err_t i2c_manager_write(
    uint8_t addr,
    uint8_t reg,
    uint8_t *data,
    size_t len)
{
    i2c_cmd_handle_t cmd =
        i2c_cmd_link_create();

    i2c_master_start(cmd);

    i2c_master_write_byte(
        cmd,
        (addr << 1) | I2C_MASTER_WRITE,
        true);

    i2c_master_write_byte(
        cmd,
        reg,
        true);

    i2c_master_write(
        cmd,
        data,
        len,
        true);

    i2c_master_stop(cmd);

    esp_err_t ret =
        i2c_master_cmd_begin(
            I2C_MASTER_NUM,
            cmd,
            pdMS_TO_TICKS(100));

    i2c_cmd_link_delete(cmd);

    return ret;
}

// =====================
// READ
// =====================

/**
 * @brief Lee datos desde un registro de un dispositivo I2C.
 *
 * La secuencia implementada es:
 *
 * START
 * -> Dirección dispositivo + Write
 * -> Registro origen
 * -> RESTART
 * -> Dirección dispositivo + Read
 * -> Datos
 * -> STOP
 *
 * @param addr Dirección I2C del dispositivo.
 * @param reg Registro origen.
 * @param data Buffer de recepción.
 * @param len Número de bytes a leer.
 *
 * @return Código de error ESP-IDF.
 */
esp_err_t i2c_manager_read(
    uint8_t addr,
    uint8_t reg,
    uint8_t *data,
    size_t len)
{
    i2c_cmd_handle_t cmd =
        i2c_cmd_link_create();

    i2c_master_start(cmd);

    i2c_master_write_byte(
        cmd,
        (addr << 1) | I2C_MASTER_WRITE,
        true);

    i2c_master_write_byte(
        cmd,
        reg,
        true);

    i2c_master_start(cmd);

    i2c_master_write_byte(
        cmd,
        (addr << 1) | I2C_MASTER_READ,
        true);

    if (len > 1) {
        i2c_master_read(
            cmd,
            data,
            len - 1,
            I2C_MASTER_ACK);
    }

    i2c_master_read_byte(
        cmd,
        data + len - 1,
        I2C_MASTER_NACK);

    i2c_master_stop(cmd);

    esp_err_t ret =
        i2c_master_cmd_begin(
            I2C_MASTER_NUM,
            cmd,
            pdMS_TO_TICKS(100));

    i2c_cmd_link_delete(cmd);

    return ret;
}

// =====================
// WRITE RAW
// =====================

/**
 * @brief Envía un bloque de datos sin especificar registro.
 *
 * Esta función es utilizada por dispositivos que requieren
 * transmisiones directas de comandos o datos, como el
 * controlador OLED SSD1306.
 *
 * @param addr Dirección I2C del dispositivo.
 * @param data Datos a transmitir.
 * @param len Cantidad de bytes.
 *
 * @return Código de error ESP-IDF.
 */
esp_err_t i2c_manager_write_raw(
    uint8_t addr,
    const uint8_t *data,
    size_t len)
{
    i2c_cmd_handle_t cmd =
        i2c_cmd_link_create();

    i2c_master_start(cmd);

    i2c_master_write_byte(
        cmd,
        (addr << 1) | I2C_MASTER_WRITE,
        true);

    i2c_master_write(
        cmd,
        (uint8_t *)data,
        len,
        true);

    i2c_master_stop(cmd);

    esp_err_t ret =
        i2c_master_cmd_begin(
            I2C_MASTER_NUM,
            cmd,
            pdMS_TO_TICKS(100));

    i2c_cmd_link_delete(cmd);

    return ret;
}