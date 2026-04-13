/*
 * @file queues.h
 * @brief Definición de colas del sistema.
 * 
 * Permite la comunicación entre tareas mediante FreeRTOS queues.
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
 * @brief Cola de datos de sensores.
 */
extern QueueHandle_t queue_sensores;

/**
 * @brief Cola de comandos de control.
 */
extern QueueHandle_t queue_control;

/**
 * @brief Cola de logging.
 */
extern QueueHandle_t queue_logger;

/**
 * @brief Inicializa todas las colas del sistema.
 */
void queues_init(void);

#endif