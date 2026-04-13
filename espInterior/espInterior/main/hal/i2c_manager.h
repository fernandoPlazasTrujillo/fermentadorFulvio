/*
 * @file i2c_manager.h
 * @brief Abstracción del bus I2C para comunicación con dispositivos.
 * 
 * Proporciona funciones de lectura y escritura, además de control
 * de concurrencia mediante mutex.
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
 */
extern SemaphoreHandle_t mutex_i2c;

/**
 * @brief Inicializa el bus I2C en modo maestro.
 */
void i2c_manager_init(void);

/**
 * @brief Escribe datos a un dispositivo I2C.
 * 
 * @param addr Dirección del dispositivo.
 * @param reg Registro interno del dispositivo.
 * @param data Datos a enviar.
 * @param len Número de bytes a enviar.
 * @return Código de error ESP.
 */
esp_err_t i2c_manager_write(uint8_t addr, uint8_t reg, uint8_t *data, size_t len);

/**
 * @brief Lee datos desde un dispositivo I2C.
 * 
 * @param addr Dirección del dispositivo.
 * @param reg Registro interno del dispositivo.
 * @param data Buffer donde se almacenarán los datos.
 * @param len Número de bytes a leer.
 * @return Código de error ESP.
 */
esp_err_t i2c_manager_read(uint8_t addr, uint8_t reg, uint8_t *data, size_t len);

#endif