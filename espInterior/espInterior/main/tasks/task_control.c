/**
 * @file task_control.c
 * @brief Implementación de la lógica de control del sistema.
 *
 * Esta tarea recibe mediciones provenientes de los sensores,
 * evalúa las condiciones del proceso y genera comandos para
 * los actuadores.
 *
 * La lógica de control se mantiene desacoplada del hardware,
 * produciendo estructuras control_cmd_t que son enviadas
 * mediante una cola FreeRTOS hacia la tarea de actuadores.
 *
 * Funciones principales:
 * - Procesamiento de datos de sensores.
 * - Evaluación de condiciones del proceso.
 * - Generación de comandos de control.
 * - Coordinación de ciclos de mezcla.
 *
 * Arquitectura:
 *
 * queue_sensores
 *        |
 *        v
 *   task_control
 *        |
 *        v
 *  queue_control
 *
 * Mecanismos FreeRTOS utilizados:
 * - Colas de comunicación entre tareas.
 *
 * @author
 * Fernando Plazas Trujillo
 * Isabella Ordoñez
 * Juan Daniel Constain
 */

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "esp_log.h"

#include "utils/data_types.h"
#include "queues.h"

/**
 * @brief Etiqueta utilizada para mensajes de depuración.
 */
static const char *TAG = "CONTROL";

/**
 * @brief Umbral de concentración de CO2.
 *
 * Utilizado para determinar condiciones específicas
 * del proceso de fermentación.
 */
#define CO2_THRESHOLD 0.02

/**
 * @brief Número de ciclos entre mezclas consecutivas.
 *
 * Cada cierto número de ciclos de adquisición se genera
 * una orden de mezcla para homogeneizar el proceso.
 */
#define MIX_INTERVAL_CYCLES 3

/**
 * @brief Tarea principal de control.
 *
 * Espera datos provenientes de queue_sensores,
 * procesa las mediciones recibidas y genera comandos
 * para los actuadores.
 *
 * La tarea implementa únicamente la lógica de decisión
 * del sistema y no accede directamente a hardware.
 *
 * Flujo de ejecución:
 * - Recibe datos de sensores.
 * - Evalúa condiciones del proceso.
 * - Genera comandos de actuación.
 * - Envía comandos a queue_control.
 *
 * @param pvParameters Parámetros de la tarea (no utilizados).
 */
void task_control(void *pvParameters)
{
    sensor_data_t data;
    control_cmd_t cmd;

    int cycle_count = 0;

    while (1)
    {
        if (xQueueReceive(queue_sensores,
                          &data,
                          portMAX_DELAY))
        {
            /**
             * Incrementar contador de ciclos.
             *
             * Se utiliza para determinar cuándo
             * debe ejecutarse una nueva mezcla.
             */
            cycle_count++;

            ESP_LOGI(TAG, "Procesando datos...");

            /**
             * Evaluación del nivel de CO2.
             *
             * Variable reservada para futuras
             * estrategias de control.
             */
            bool aire_activo = (data.co2 > CO2_THRESHOLD);

            (void)aire_activo;

            /**
             * Programar mezcla periódica.
             */
            cmd.mezclar =
                (cycle_count % MIX_INTERVAL_CYCLES == 0);

            /**
             * Lógica de enfriamiento.
             *
             * Actualmente deshabilitada.
             */
            cmd.enfriar = false;

            /**
             * La bomba sigue el estado del sistema
             * de enfriamiento.
             */
            cmd.bomba = cmd.enfriar;

            /**
             * Posición por defecto del servomotor.
             */
            cmd.servo_angle = 0.0f;

            ESP_LOGI(TAG, "Ciclo: %d", cycle_count);
            ESP_LOGI(TAG, "Mezclar: %d", cmd.mezclar);

            xQueueSend(queue_control,
                       &cmd,
                       portMAX_DELAY);
        }
    }
}