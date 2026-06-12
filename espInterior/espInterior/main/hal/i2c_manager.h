/**
 * @file i2c_manager.h
 * @brief Capa de abstracción para el bus I2C del ESP32.
 *
 * Este módulo proporciona una interfaz común para la comunicación
 * con dispositivos conectados al bus I2C.
 *
 * Funcionalidades:
 * - Inicialización del bus I2C en modo maestro.
 * - Lectura de registros.
 * - Escritura de registros.
 * - Escritura de bloques de datos.
 * - Protección mediante mutex para acceso concurrente.
 *
 * Los siguientes módulos utilizan esta capa:
 * - rtc_ds3231
 * - oled_display
 *
 * Esta abstracción permite desacoplar los drivers de los detalles
 * específicos de ESP-IDF y facilita el mantenimiento del sistema.
 *
 * @author
 * Fernando Plazas Trujillo
 * Isabella Ordoñez
 * Juan Daniel Constain
 */

#ifndef I2C_MANAGER_H
#define I2C_MANAGER_H

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

#include "esp_err.h"

/**
 * @brief Mutex global para acceso seguro al bus I2C.
 *
 * Garantiza exclusión mutua cuando múltiples tareas
 * acceden simultáneamente a dispositivos conectados
 * al mismo bus.
 */
extern SemaphoreHandle_t mutex_i2c;

/**
 * @brief Inicializa el bus I2C en modo maestro.
 *
 * Configura los pines SDA y SCL, la frecuencia del bus
 * y crea los recursos necesarios para la comunicación.
 *
 * Esta función debe ejecutarse una única vez durante
 * la inicialización del sistema.
 */
void i2c_manager_init(void);

/**
 * @brief Escribe datos en un registro de un dispositivo I2C.
 *
 * Realiza una transacción de escritura compuesta por:
 * - Dirección del dispositivo.
 * - Dirección del registro.
 * - Datos asociados.
 *
 * @param addr Dirección I2C del dispositivo.
 * @param reg Registro interno de destino.
 * @param data Buffer con los datos a transmitir.
 * @param len Número de bytes a escribir.
 *
 * @return
 * - ESP_OK: Operación exitosa.
 * - Código de error en caso contrario.
 */
esp_err_t i2c_manager_write(
    uint8_t addr,
    uint8_t reg,
    uint8_t *data,
    size_t len);

/**
 * @brief Lee datos desde un registro de un dispositivo I2C.
 *
 * Realiza una transacción de lectura a partir del
 * registro especificado.
 *
 * @param addr Dirección I2C del dispositivo.
 * @param reg Registro interno a leer.
 * @param data Buffer donde se almacenarán los datos.
 * @param len Número de bytes a recibir.
 *
 * @return
 * - ESP_OK: Operación exitosa.
 * - Código de error en caso contrario.
 */
esp_err_t i2c_manager_read(
    uint8_t addr,
    uint8_t reg,
    uint8_t *data,
    size_t len);

/**
 * @brief Envía un bloque de datos sin especificar registro.
 *
 * Esta función es utilizada por dispositivos que requieren
 * transmisiones directas de comandos o datos, como la
 * pantalla OLED SSD1306.
 *
 * @param addr Dirección I2C del dispositivo.
 * @param data Datos a transmitir.
 * @param len Número de bytes a enviar.
 *
 * @return
 * - ESP_OK: Operación exitosa.
 * - Código de error en caso contrario.
 */
esp_err_t i2c_manager_write_raw(
    uint8_t addr,
    const uint8_t *data,
    size_t len);

#endif