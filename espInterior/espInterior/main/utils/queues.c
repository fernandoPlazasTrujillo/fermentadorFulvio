/**
 * @file queues.c
 * @brief Implementación de las colas del sistema.
 * 
 * Crea e inicializa las colas utilizadas para la comunicación
 * entre tareas en FreeRTOS.
 * 
 * Las colas permiten desacoplar la adquisición, control y registro.
 * 
 * @author
 * Fernando Plazas Trujillo
 * Isabella Ordoñez
 * Juan Daniel Constain
 */

#include "queues.h"

/** @brief Cola de sensores */
QueueHandle_t queue_sensores;

/** @brief Cola de control */
QueueHandle_t queue_control;

/** @brief Cola de logger */
QueueHandle_t queue_logger;

/** @brief Cola de MQTT */
QueueHandle_t queue_mqtt;

/** @brief Cola  de display*/
QueueHandle_t queue_display;

/**
 * @brief Inicializa las colas del sistema.
 */
void queues_init(void)
{
    queue_sensores = xQueueCreate(5, sizeof(sensor_data_t));
    queue_control  = xQueueCreate(5, sizeof(control_cmd_t));
    queue_logger   = xQueueCreate(5, sizeof(sensor_data_t));
    queue_mqtt     = xQueueCreate(10, sizeof(sensor_data_t));
    queue_display  = xQueueCreate(5, sizeof(display_data_t));
    
}
