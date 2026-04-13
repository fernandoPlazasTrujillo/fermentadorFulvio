/**
 * @file task_actuadores.c
 * @brief Control de actuadores del sistema.
 * 
 * Esta tarea recibe comandos desde la cola de control y ejecuta
 * acciones sobre bomba, motor y servomotor.
 * 
 * Incluye control de duración de mezcla y sincronización con la
 * tarea de energía.
 * 
 * @author
 * Fernando Plazas Trujillo
 * Isabella Ordoñez
 * Juan Daniel Constain
 */

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"

#include "utils/data_types.h"
#include "queues.h"
#include "drivers/servo.h"

// sincronización con energía
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
#define MIX_DURATION_MS 20000

// =====================
// INIT GPIO + SERVO
// =====================

/**
 * @brief Inicializa los actuadores.
 */
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

    gpio_set_level(PIN_BOMBA, 0);
    gpio_set_level(PIN_MOTOR, 0);

    servo_init(PIN_SERVO);
}

// =====================
// TASK ACTUADORES
// =====================

/**
 * @brief Tarea principal de actuadores.
 */
void task_actuadores(void *pvParameters)
{
    control_cmd_t cmd;

    actuadores_init();

    bool motor_activo = false;
    TickType_t motor_start_time = 0;

    float last_servo_angle = -1;

    ESP_LOGI(TAG, "Task actuadores iniciada");

    while (1) {

        if (xQueueReceive(queue_control, &cmd, pdMS_TO_TICKS(100))) {

            ESP_LOGI(TAG, "Ejecutando acciones...");

            gpio_set_level(PIN_BOMBA, cmd.bomba ? 1 : 0);

            if (cmd.servo_angle != last_servo_angle) {
                servo_set_angle(cmd.servo_angle);
                last_servo_angle = cmd.servo_angle;
            }

            if (cmd.mezclar && !motor_activo) {

                ESP_LOGI(TAG, "Iniciando mezcla");

                gpio_set_level(PIN_MOTOR, 1);

                motor_activo = true;
                motor_start_time = xTaskGetTickCount();
            }
        }

        if (motor_activo) {

            TickType_t now = xTaskGetTickCount();

            if ((now - motor_start_time) >= pdMS_TO_TICKS(MIX_DURATION_MS)) {

                ESP_LOGI(TAG, "Deteniendo mezcla");

                gpio_set_level(PIN_MOTOR, 0);
                motor_activo = false;

                xTaskNotifyGive(task_energia_handle);
            }
        }

        vTaskDelay(pdMS_TO_TICKS(50));
    }
}