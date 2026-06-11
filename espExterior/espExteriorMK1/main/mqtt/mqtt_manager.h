/**
 * @file mqtt_manager.h
 * @brief Gestión de la comunicación MQTT del sistema.
 *
 * Este módulo encapsula la inicialización, conexión,
 * publicación y detención del cliente MQTT utilizado
 * por el sistema de monitoreo de fermentación.
 *
 * Permite que otras tareas de FreeRTOS publiquen datos
 * hacia el broker MQTT sin depender directamente de la
 * implementación interna del protocolo.
 */

#ifndef MQTT_MANAGER_H
#define MQTT_MANAGER_H

#include <stdbool.h>
#include <stdint.h>

/**
 * @brief Inicializa y arranca el cliente MQTT.
 *
 * Configura la conexión con el broker MQTT y registra
 * los eventos necesarios para la gestión de la comunicación.
 */
void mqtt_manager_start(void);

/**
 * @brief Espera a que el cliente MQTT establezca conexión.
 *
 * La función bloquea hasta que se establezca la conexión
 * o hasta que expire el tiempo máximo especificado.
 *
 * @param timeout_ms Tiempo máximo de espera en milisegundos.
 *
 * @return true si la conexión fue establecida.
 * @return false si ocurrió un timeout.
 */
bool mqtt_wait_connected(uint32_t timeout_ms);

/**
 * @brief Publica un mensaje en un tópico MQTT.
 *
 * Envía un mensaje al broker utilizando el tópico indicado.
 *
 * @param topic Tópico de destino.
 * @param payload Contenido del mensaje.
 * @param qos Nivel de calidad de servicio MQTT.
 * @param timeout_ms Tiempo máximo de espera para la publicación.
 *
 * @return true si la publicación fue exitosa.
 * @return false si ocurrió un error o timeout.
 */
bool mqtt_publish(const char *topic,
                  const char *payload,
                  int qos,
                  uint32_t timeout_ms);

/**
 * @brief Detiene el cliente MQTT.
 *
 * Finaliza la conexión con el broker y libera los recursos
 * asociados al cliente MQTT.
 */
void mqtt_manager_stop(void);

#endif