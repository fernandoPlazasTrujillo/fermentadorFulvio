/**
 * @file queues.h
 * @brief Declaración de las colas de comunicación del sistema.
 *
 * Este módulo define las colas FreeRTOS utilizadas para el
 * intercambio de información entre las diferentes tareas
 * del sistema embebido.
 *
 * Las colas permiten una comunicación segura y desacoplada
 * entre productores y consumidores, evitando el uso de
 * variables globales compartidas.
 *
 * Flujo principal:
 *
 * task_sensores
 *      |
 *      v
 * queue_sensores
 *      |
 *      v
 * task_control
 *      |
 *      v
 * queue_control
 *      |
 *      v
 * task_actuadores
 *
 * Además, los datos pueden ser distribuidos hacia:
 * - queue_logger
 * - queue_display
 * - queue_mqtt
 *
 * @author
 * Fernando Plazas Trujillo
 * Isabella Ordoñez
 * Juan Daniel Constain
 */

#ifndef QUEUES_H
#define QUEUES_H

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

#include "utils/data_types.h"

/**
 * @brief Cola de datos provenientes de los sensores.
 *
 * Transporta estructuras sensor_data_t desde la tarea
 * de adquisición hacia las demás tareas consumidoras.
 */
extern QueueHandle_t queue_sensores;

/**
 * @brief Cola de comandos de control.
 *
 * Utilizada para enviar estructuras control_cmd_t
 * desde la tarea de control hacia la tarea de actuadores.
 */
extern QueueHandle_t queue_control;

/**
 * @brief Cola para registro de datos.
 *
 * Permite transferir mediciones hacia la tarea encargada
 * del almacenamiento en tarjeta SD.
 */
extern QueueHandle_t queue_logger;

/**
 * @brief Cola para publicación MQTT.
 *
 * Permite enviar información desde las tareas de aplicación
 * hacia la tarea responsable de la comunicación MQTT.
 */
extern QueueHandle_t queue_mqtt;

/**
 * @brief Cola de actualización de pantalla.
 *
 * Transporta estructuras display_data_t hacia la tarea
 * encargada de la interfaz de usuario.
 */
extern QueueHandle_t queue_display;

/**
 * @brief Inicializa todas las colas del sistema.
 *
 * Crea las colas FreeRTOS necesarias para la comunicación
 * entre tareas y verifica la correcta asignación de memoria.
 */
void queues_init(void);

#endif /* QUEUES_H */