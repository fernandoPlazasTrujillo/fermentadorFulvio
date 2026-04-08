#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"

#include "utils/data_types.h"
#include "queues.h"
#include "drivers/servo.h"

// 🔥 sincronización con energía
extern TaskHandle_t task_energia_handle;

static const char *TAG = "ACTUADORES";

// =====================
// PINES
// =====================
#define PIN_BOMBA  GPIO_NUM_25
#define PIN_MOTOR  GPIO_NUM_26
#define PIN_SERVO  GPIO_NUM_27

// =====================
// CONFIGURACIÓN
// =====================
#define MIX_DURATION_MS 20000   // 20 segundos

// =====================
// INIT GPIO + SERVO
// =====================
static void actuadores_init(void)
{
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << PIN_BOMBA) | (1ULL << PIN_MOTOR),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };

    gpio_config(&io_conf);

    // Estado inicial
    gpio_set_level(PIN_BOMBA, 0);
    gpio_set_level(PIN_MOTOR, 0);

    // 🔥 INIT SERVO
    servo_init(PIN_SERVO);
}

// =====================
// TASK ACTUADORES
// =====================
void task_actuadores(void *pvParameters)
{
    control_cmd_t cmd;

    actuadores_init();

    // =====================
    // ESTADOS INTERNOS
    // =====================
    bool motor_activo = false;
    TickType_t motor_start_time = 0;

    float last_servo_angle = -1;

    ESP_LOGI(TAG, "Task actuadores iniciada");

    while (1) {

        // =====================
        // RECIBIR COMANDOS
        // =====================
        if (xQueueReceive(queue_control, &cmd, pdMS_TO_TICKS(100))) {

            ESP_LOGI(TAG, "Ejecutando acciones...");

            // =====================
            // BOMBA
            // =====================
            gpio_set_level(PIN_BOMBA, cmd.bomba ? 1 : 0);
            ESP_LOGI(TAG, "Bomba: %d", cmd.bomba);

            // =====================
            // SERVO (PWM)
            // =====================
            if (cmd.servo_angle != last_servo_angle) {

                servo_set_angle(cmd.servo_angle);
                last_servo_angle = cmd.servo_angle;

                ESP_LOGI(TAG, "Servo: %.2f grados", cmd.servo_angle);
            }

            // =====================
            // INICIO DE MEZCLA (MOTOR)
            // =====================
            if (cmd.mezclar && !motor_activo) {

                ESP_LOGI(TAG, "Iniciando mezcla (20s)");

                gpio_set_level(PIN_MOTOR, 1);

                motor_activo = true;
                motor_start_time = xTaskGetTickCount();
            }
        }

        // =====================
        // CONTROL DE DURACIÓN MOTOR
        // =====================
        if (motor_activo) {

            TickType_t now = xTaskGetTickCount();

            if ((now - motor_start_time) >= pdMS_TO_TICKS(MIX_DURATION_MS)) {

                ESP_LOGI(TAG, "Deteniendo mezcla");

                gpio_set_level(PIN_MOTOR, 0);
                motor_activo = false;

                // 🔥 Notificar a energía
                xTaskNotifyGive(task_energia_handle);
            }
        }

        // =====================
        // DELAY PEQUEÑO
        // =====================
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}