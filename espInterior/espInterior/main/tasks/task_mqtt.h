/**
 * @file task_mqtt.h
 * @brief Declaración de la tarea de comunicación MQTT.
 *
 * Este módulo define la tarea responsable de publicar los
 * datos del sistema en un broker MQTT para monitoreo remoto.
 *
 * La tarea recibe mediciones provenientes de los sensores
 * mediante una cola FreeRTOS y realiza su envío utilizando
 * el módulo mqtt_manager.
 *
 * Funciones principales:
 * - Recepción de datos de sensores.
 * - Formateo de mensajes MQTT.
 * - Publicación en tópicos MQTT.
 * - Monitoreo del estado de conexión.
 *
 * Arquitectura:
 *
 * queue_mqtt
 *       |
 *       v
 *   task_mqtt
 *       |
 *       v
 * Broker MQTT
 *
 * Esta tarea desacopla las comunicaciones de red de las
 * tareas de adquisición y control del sistema.
 *
 * @author
 * Fernando Plazas Trujillo
 * Isabella Ordoñez
 * Juan Daniel Constain
 */

#ifndef TASK_MQTT_H
#define TASK_MQTT_H

/**
 * @brief Tarea principal de publicación MQTT.
 *
 * Espera datos provenientes de queue_mqtt, genera los
 * mensajes correspondientes y los publica en el broker MQTT.
 *
 * La tarea opera de forma independiente para evitar que
 * las latencias de red afecten la adquisición de sensores
 * o la ejecución de la lógica de control.
 *
 * @param pvParameters Parámetros de la tarea (no utilizados).
 */
void task_mqtt(void *pvParameters);

#endif /* TASK_MQTT_H */