/**
 * @file queues.h
 * @brief Declaración de las colas FreeRTOS utilizadas por el sistema.
 *
 * Este archivo centraliza las colas empleadas para la comunicación
 * entre tareas del ESP32 externo.
 *
 * Las colas permiten desacoplar la adquisición de datos,
 * la visualización y la transmisión de información,
 * siguiendo una arquitectura basada en paso de mensajes.
 *
 * @section queue_architecture Arquitectura de comunicación
 *
 * Flujo principal:
 *
 * task_sensores
 *      |
 *      +--> cola_datos
 *      |
 *      +--> cola_display
 *      |
 *      +--> cola_mqtt
 *
 * De esta manera las tareas no comparten variables globales
 * directamente, reduciendo problemas de sincronización.
 *
 * @note Las colas son creadas durante la inicialización del sistema
 * y posteriormente utilizadas por las distintas tareas FreeRTOS.
 *
 * @authors
 * - Fernando Plazas
 * - Isabella Ordoñez
 * - Juan Daniel Constain
 */

#pragma once

#include "freertos/queue.h"
#include "common.h"

/**
 * @brief Cola de datos para procesamiento interno.
 *
 * Transporta estructuras de tipo datos_ambiente_t entre
 * las tareas encargadas de adquisición y procesamiento.
 */
extern QueueHandle_t cola_datos;

/**
 * @brief Cola utilizada para actualización de la interfaz gráfica.
 *
 * Permite enviar nuevas mediciones hacia la tarea encargada
 * de controlar la pantalla LCD.
 */
extern QueueHandle_t cola_display;

/**
 * @brief Cola utilizada para transmisión MQTT.
 *
 * Contiene los datos que posteriormente serán serializados
 * y publicados hacia el broker MQTT.
 */
extern QueueHandle_t cola_mqtt;