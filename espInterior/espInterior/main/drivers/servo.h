#ifndef SERVO_H
#define SERVO_H

#include "esp_err.h"
#include "driver/gpio.h"

esp_err_t servo_init(gpio_num_t pin);
esp_err_t servo_set_angle(float angle);

#endif