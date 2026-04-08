#ifndef DS18B20_H
#define DS18B20_H

#include "esp_err.h"
#include "driver/gpio.h" 

esp_err_t ds18b20_init(gpio_num_t pin);
float ds18b20_read_temperature(void);

#endif