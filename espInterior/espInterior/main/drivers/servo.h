/**
 * @file servo.h
 * @brief Control de servomotor mediante PWM usando LEDC del ESP32.
 * 
 * Permite inicializar el servo y posicionarlo en un ángulo específico.
 * 
 * @author
 * Fernando Plazas Trujillo
 * Isabella Ordoñez
 * Juan Daniel Constain
 */

#ifndef SERVO_H
#define SERVO_H

#include "esp_err.h"
#include "driver/gpio.h"

/**
 * @brief Inicializa el servomotor.
 * 
 * Configura el temporizador y canal PWM (LEDC).
 * 
 * @param pin GPIO donde está conectado el servo.
 * @return ESP_OK si la inicialización fue exitosa.
 */
esp_err_t servo_init(gpio_num_t pin);

/**
 * @brief Establece el ángulo del servomotor.
 * 
 * Convierte el ángulo (0–180°) a una señal PWM.
 * 
 * @param angle Ángulo en grados (0 a 180).
 * @return ESP_OK si la operación fue exitosa.
 */
esp_err_t servo_set_angle(float angle);

#endif