/**
 * @file task_actuadores.c
 * @brief Implementación de la tarea de ejecución de actuadores.
 *
 * Esta tarea recibe comandos generados por la lógica de control
 * mediante una cola FreeRTOS y ejecuta las acciones físicas
 * correspondientes sobre los actuadores del sistema.
 *
 * Actuadores gestionados:
 * - Bomba de circulación.
 * - Motor de mezcla.
 * - Servomotor.
 *
 * Características:
 * - Consumo de comandos mediante colas FreeRTOS.
 * - Control temporizado del motor de mezcla.
 * - Notificación a la tarea de energía al finalizar una mezcla.
 * - Separación entre lógica de control y acceso a hardware.
 *
 * Mecanismos FreeRTOS utilizados:
 * - Colas.
 * - Notificaciones entre tareas.
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

/**
 * @brief Handle de la tarea de energía.
 *
 * Utilizado para enviar notificaciones cuando finaliza
 * un ciclo de mezcla.
 */
extern TaskHandle_t task_energia_handle;

/**
 * @brief Etiqueta utilizada para mensajes de depuración.
 */
static const char *TAG = "ACTUADORES";

// =====================
// PINES
// =====================
/**
 * @brief GPIO de control de la bomba.
 */
#define PIN_BOMBA GPIO_NUM_25

/**
 * @brief GPIO de control del motor de mezcla.
 */
#define PIN_MOTOR GPIO_NUM_26

/**
 * @brief GPIO utilizado por el servomotor.
 */
#define PIN_SERVO GPIO_NUM_27

/**
 * @brief Duración del ciclo de mezcla.
 *
 * Tiempo durante el cual el motor permanece activo.
 */
#define MIX_DURATION_MS 20000

// =====================
// INIT GPIO + SERVO
// =====================

/**
 * @brief Inicializa los actuadores del sistema.
 *
 * Configura los GPIO utilizados por la bomba y el motor
 * en modo salida e inicializa el controlador del servomotor.
 *
 * Además establece todos los actuadores en estado seguro
 * al arrancar el sistema.
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


/**
 * @brief Tarea encargada de ejecutar acciones físicas.
 *
 * Esta tarea permanece bloqueada esperando comandos
 * provenientes de queue_control.
 *
 * Funciones principales:
 * - Activación y desactivación de la bomba.
 * - Posicionamiento del servomotor.
 * - Gestión del motor de mezcla.
 * - Notificación a task_energia al finalizar una mezcla.
 *
 * La tarea no implementa lógica de decisión; únicamente
 * ejecuta los comandos generados por task_control.
 *
 * @param pvParameters Parámetros de la tarea (no utilizados).
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