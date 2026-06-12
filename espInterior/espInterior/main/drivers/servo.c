/**
 * @file servo.c
 * @brief Implementación del control de servomotor usando PWM mediante LEDC.
 *
 * Este módulo genera señales PWM compatibles con servomotores
 * estándar utilizando el periférico LEDC del ESP32.
 *
 * Características:
 * - Frecuencia de operación de 50 Hz.
 * - Resolución PWM de 16 bits.
 * - Conversión automática de ángulo a ciclo de trabajo.
 *
 * El servomotor puede utilizarse para accionar mecanismos
 * físicos dentro del sistema de fermentación, como sistemas
 * de mezcla o posicionamiento.
 *
 * @author
 * Fernando Plazas Trujillo
 * Isabella Ordoñez
 * Juan Daniel Constain
 */

#include "drivers/servo.h"
#include "driver/ledc.h"
#include "esp_log.h"

/**
 * @brief Etiqueta utilizada por el sistema de logging.
 */
static const char *TAG = "SERVO";

/**
 * @brief Frecuencia PWM utilizada por el servomotor.
 *
 * Los servomotores convencionales utilizan una frecuencia
 * de aproximadamente 50 Hz (periodo de 20 ms).
 */
#define SERVO_FREQ 50

/**
 * @brief Temporizador LEDC utilizado.
 */
#define SERVO_TIMER LEDC_TIMER_0

/**
 * @brief Modo de operación LEDC.
 */
#define SERVO_MODE LEDC_HIGH_SPEED_MODE

/**
 * @brief Canal PWM utilizado.
 */
#define SERVO_CHANNEL LEDC_CHANNEL_0

/**
 * @brief Resolución PWM configurada.
 */
#define SERVO_RESOLUTION LEDC_TIMER_16_BIT

/**
 * @brief Ciclo de trabajo máximo para resolución de 16 bits.
 */
#define SERVO_MAX_DUTY 65535

/**
 * @brief Periodo PWM en microsegundos.
 *
 * 50 Hz = 20 ms = 20000 us
 */
#define SERVO_PERIOD_US 20000.0f

/**
 * @brief Ancho de pulso correspondiente a 0°.
 */
#define SERVO_MIN_PULSE_US 1000.0f

/**
 * @brief Ancho de pulso correspondiente a 180°.
 */
#define SERVO_MAX_PULSE_US 2000.0f

/**
 * @brief GPIO utilizado para el servomotor.
 */
static gpio_num_t servo_pin;

// =====================
// INIT
// =====================

/**
 * @brief Inicializa el servomotor.
 *
 * Configura el temporizador LEDC y el canal PWM asociado
 * al GPIO indicado.
 *
 * @param pin GPIO conectado al servomotor.
 *
 * @return
 * - ESP_OK: Inicialización exitosa.
 * - Código de error en caso contrario.
 */
esp_err_t servo_init(gpio_num_t pin)
{
    servo_pin = pin;

    ledc_timer_config_t timer = {
        .speed_mode = SERVO_MODE,
        .timer_num = SERVO_TIMER,
        .duty_resolution = SERVO_RESOLUTION,
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
 * @brief Posiciona el servomotor en el ángulo especificado.
 *
 * Convierte el ángulo solicitado en un ancho de pulso PWM:
 *
 * - 0°   -> 1.0 ms
 * - 90°  -> 1.5 ms
 * - 180° -> 2.0 ms
 *
 * Posteriormente el ancho de pulso se transforma al valor
 * de duty cycle requerido por el periférico LEDC.
 *
 * @param angle Ángulo deseado en grados.
 *
 * @return
 * - ESP_OK: Operación realizada correctamente.
 */
esp_err_t servo_set_angle(float angle)
{
    if (angle < 0.0f) {
        angle = 0.0f;
    }

    if (angle > 180.0f) {
        angle = 180.0f;
    }

    /*
     * Conversión lineal de ángulo a ancho de pulso.
     */
    float pulse_us =
        SERVO_MIN_PULSE_US +
        ((angle / 180.0f) *
        (SERVO_MAX_PULSE_US - SERVO_MIN_PULSE_US));

    /*
     * Conversión de ancho de pulso a duty cycle.
     */
    uint32_t duty =
        (uint32_t)((pulse_us / SERVO_PERIOD_US) *
        SERVO_MAX_DUTY);

    ledc_set_duty(
        SERVO_MODE,
        SERVO_CHANNEL,
        duty);

    ledc_update_duty(
        SERVO_MODE,
        SERVO_CHANNEL);

    ESP_LOGI(
        TAG,
        "Servo angle: %.2f deg | pulse: %.2f us | duty: %lu",
        angle,
        pulse_us,
        (unsigned long)duty);

    return ESP_OK;
}