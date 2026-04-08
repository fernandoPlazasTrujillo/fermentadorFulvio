#ifndef I2C_MANAGER_H
#define I2C_MANAGER_H

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "esp_err.h"

// 🔥 DECLARACIÓN GLOBAL
extern SemaphoreHandle_t mutex_i2c;

void i2c_manager_init(void);

esp_err_t i2c_manager_write(uint8_t addr, uint8_t reg, uint8_t *data, size_t len);
esp_err_t i2c_manager_read(uint8_t addr, uint8_t reg, uint8_t *data, size_t len);

#endif