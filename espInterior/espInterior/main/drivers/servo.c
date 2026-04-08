#include "drivers/servo.h"
#include "driver/ledc.h"
#include "esp_log.h"

static const char *TAG = "SERVO";

#define SERVO_FREQ 50
#define SERVO_TIMER LEDC_TIMER_0
#define SERVO_MODE LEDC_HIGH_SPEED_MODE
#define SERVO_CHANNEL LEDC_CHANNEL_0

static gpio_num_t servo_pin;

// =====================
// INIT
// =====================
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
// ANGLE → DUTY
// =====================
esp_err_t servo_set_angle(float angle)
{
    if (angle < 0) angle = 0;
    if (angle > 180) angle = 180;

    // duty en microsegundos
    float pulse_us = 1000 + (angle / 180.0) * 1000; // 1000–2000us

    // periodo = 20ms → 20000us
    uint32_t duty = (pulse_us / 20000.0) * 65535;

    ledc_set_duty(SERVO_MODE, SERVO_CHANNEL, duty);
    ledc_update_duty(SERVO_MODE, SERVO_CHANNEL);

    ESP_LOGI(TAG, "Servo angle: %.2f", angle);

    return ESP_OK;
}