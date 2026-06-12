/**
 * @file queues.c
 * @brief Implementación de las colas de comunicación del sistema.
 *
 * Este módulo crea e inicializa las colas FreeRTOS utilizadas
 * para el intercambio de información entre las distintas tareas
 * del sistema.
 *
 * Las colas permiten desacoplar la adquisición de datos,
 * la lógica de control, la visualización, el almacenamiento
 * y la comunicación MQTT.
 *
 * Arquitectura de comunicación:
 *
 * task_sensores
 *      |
 *      +----> queue_sensores
 *      +----> queue_logger
 *      +----> queue_mqtt
 *      +----> queue_display
 *
 * task_control
 *      |
 *      +----> queue_control
 *
 * Las colas garantizan una comunicación segura entre tareas
 * sin necesidad de compartir variables globales.
 *
 * @author
 * Fernando Plazas Trujillo
 * Isabella Ordoñez
 * Juan Daniel Constain
 */

#include "queues.h"

/**
 * @brief Cola de datos provenientes de sensores.
 *
 * Transporta estructuras sensor_data_t entre las tareas
 * de adquisición y procesamiento.
 */
QueueHandle_t queue_sensores;

/**
 * @brief Cola de comandos de control.
 *
 * Transporta estructuras control_cmd_t desde la tarea
 * de control hacia la tarea de actuadores.
 */
QueueHandle_t queue_control;

/**
 * @brief Cola para almacenamiento de datos.
 *
 * Utilizada para transferir mediciones hacia la tarea
 * encargada del registro en tarjeta SD.
 */
QueueHandle_t queue_logger;

/**
 * @brief Cola para publicación MQTT.
 *
 * Permite enviar datos hacia la tarea responsable
 * de la comunicación con el broker MQTT.
 */
QueueHandle_t queue_mqtt;

/**
 * @brief Cola de actualización de pantalla.
 *
 * Transporta estructuras display_data_t hacia la tarea
 * encargada de la interfaz de usuario.
 */
QueueHandle_t queue_display;

/**
 * @brief Inicializa todas las colas del sistema.
 *
 * Crea las colas FreeRTOS utilizadas para la comunicación
 * entre tareas y reserva la memoria necesaria para cada una.
 *
 * Tamaños configurados:
 * - queue_sensores : 5 elementos.
 * - queue_control  : 5 elementos.
 * - queue_logger   : 5 elementos.
 * - queue_mqtt     : 10 elementos.
 * - queue_display  : 5 elementos.
 */
void queues_init(void)
{
    queue_sensores = xQueueCreate(5, sizeof(sensor_data_t));

    queue_control  = xQueueCreate(5, sizeof(control_cmd_t));

    queue_logger   = xQueueCreate(5, sizeof(sensor_data_t));

    queue_mqtt     = xQueueCreate(10, sizeof(sensor_data_t));

    queue_display  = xQueueCreate(5, sizeof(display_data_t));
}