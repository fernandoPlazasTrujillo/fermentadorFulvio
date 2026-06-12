/**
 * @file servo.h
 * @brief Driver para control de servomotores mediante PWM usando el periférico LEDC del ESP32.
 *
 * Este módulo proporciona las funciones necesarias para controlar
 * un servomotor estándar mediante señales PWM generadas por el
 * periférico LEDC del ESP32.
 *
 * Funcionalidades:
 * - Inicialización del canal PWM.
 * - Configuración del temporizador LEDC.
 * - Posicionamiento angular del servomotor.
 *
 * El módulo es utilizado para accionar mecanismos físicos dentro
 * del sistema de fermentación, como mezcla o posicionamiento
 * de elementos mecánicos.
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
 * Configura el temporizador y canal PWM del periférico LEDC
 * necesarios para generar la señal de control del servo.
 *
 * Esta función debe ejecutarse una única vez antes de utilizar
 * cualquier otra función del módulo.
 *
 * @param pin GPIO al que se encuentra conectado el servomotor.
 *
 * @return
 * - ESP_OK: Inicialización exitosa.
 * - Código de error en caso contrario.
 */
esp_err_t servo_init(gpio_num_t pin);

/**
 * @brief Posiciona el servomotor en un ángulo específico.
 *
 * Convierte el ángulo solicitado a un ciclo de trabajo PWM
 * compatible con servomotores estándar de 50 Hz.
 *
 * El rango válido de operación es de 0° a 180°.
 *
 * @param angle Ángulo deseado en grados.
 *
 * @return
 * - ESP_OK: Operación realizada correctamente.
 * - ESP_ERR_INVALID_ARG: Ángulo fuera de rango.
 */
esp_err_t servo_set_angle(float angle);

#endif