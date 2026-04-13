/**
 * @file servo.c
 * @brief Implementación del control de servomotor usando PWM (LEDC).
 * 
 * Este módulo genera señales PWM con un periodo de 20 ms (50 Hz),
 * típicas para servomotores, y ajusta el duty cycle según el ángulo.
 * 
 * @author
 * Fernando Plazas Trujillo
 * Isabella Ordoñez
 * Juan Daniel Constain
 */

#include "drivers/servo.h"
#include "driver/ledc.h"
#include "esp_log.h"

static const char *TAG = "SERVO";

/** @brief Frecuencia del PWM para servo (50 Hz) */
#define SERVO_FREQ 50

/** @brief Temporizador LEDC */
#define SERVO_TIMER LEDC_TIMER_0

/** @brief Modo de alta velocidad LEDC */
#define SERVO_MODE LEDC_HIGH_SPEED_MODE

/** @brief Canal LEDC */
#define SERVO_CHANNEL LEDC_CHANNEL_0

/** @brief Pin GPIO del servo */
static gpio_num_t servo_pin;

// =====================
// INIT
// =====================

/**
 * @brief Inicializa el servomotor.
 * 
 * Configura el temporizador y canal PWM.
 * 
 * @param pin GPIO del servo.
 * @return ESP_OK si se configura correctamente.
 */
esp_err_t servo_init(gpio_num_t pin)
{
    servo_pin = pin;

    ledc_timer_config_t timer = {
        .speed_mode = SERVO_MODE,
        .timer_num = SERVO_TIMER,
        .duty_resolution = LEDC_TIMER_16_BIT,
        .freq_hz = SERVO_FREQ,
        .clk_cfg = LEDC_AUTO_CLK
    };
    ledc_timer_config(&timer);

    ledc_channel_config_t channel = {
        .gpio_num = servo_pin,
        .speed_mode = SERVO_MODE,
        .channel = SERVO_CHANNEL,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = SERVO_TIMER,
        .duty = 0,
        .hpoint = 0
    };
    ledc_channel_config(&channel);

    ESP_LOGI(TAG, "Servo inicializado en GPIO %d", pin);

    return ESP_OK;
}

// =====================
// SET ANGLE
// =====================

/**
 * @brief Establece el ángulo del servomotor.
 * 
 * Convierte el ángulo en un ancho de pulso entre:
 * - 1 ms (0°)
 * - 2 ms (180°)
 * 
 * @param angle Ángulo deseado en grados.
 * @return ESP_OK si la operación fue exitosa.
 */
esp_err_t servo_set_angle(float angle)
{
    if (angle < 0) angle = 0;
    if (angle > 180) angle = 180;

    float pulse_us = 1000 + (angle / 180.0) * 1000;

    uint32_t duty = (pulse_us / 20000.0) * 65535;

    ledc_set_duty(SERVO_MODE, SERVO_CHANNEL, duty);
    ledc_update_duty(SERVO_MODE, SERVO_CHANNEL);

    ESP_LOGI(TAG, "Servo angle: %.2f", angle);

    return ESP_OK;
}