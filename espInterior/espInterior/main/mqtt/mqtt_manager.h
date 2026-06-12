/**
 * @file mqtt_manager.h
 * @brief Interfaz para la gestión de comunicación MQTT.
 *
 * Este módulo encapsula la configuración y operación del cliente MQTT
 * utilizado por el sistema de monitoreo ambiental basado en ESP32.
 *
 * Permite:
 * - Inicializar la conexión MQTT.
 * - Verificar el estado de conexión con el broker.
 * - Publicar mensajes en tópicos específicos.
 * - Detener el servicio MQTT.
 * - Consultar el estado actual de conectividad.
 *
 * El módulo abstrae los detalles de la biblioteca MQTT utilizada,
 * proporcionando una interfaz sencilla para las tareas de aplicación.
 *
 * @author
 * Fernando Plazas Trujillo
 * Isabella Ordoñez
 * Juan Daniel Constain
 */

#ifndef MQTT_MANAGER_H
#define MQTT_MANAGER_H

#include <stdbool.h>
#include <stdint.h>

/**
 * @brief Inicializa y pone en marcha el cliente MQTT.
 *
 * Configura el cliente MQTT, registra los eventos asociados y
 * establece la conexión con el broker configurado.
 */
void mqtt_manager_start(void);

/**
 * @brief Espera hasta que se establezca la conexión MQTT.
 *
 * Bloquea la ejecución hasta que el cliente MQTT se conecte
 * al broker o hasta que expire el tiempo de espera.
 *
 * @param timeout_ms Tiempo máximo de espera en milisegundos.
 *
 * @return true si la conexión se estableció correctamente.
 * @return false si ocurrió un timeout o error de conexión.
 */
bool mqtt_wait_connected(uint32_t timeout_ms);

/**
 * @brief Publica un mensaje en un tópico MQTT.
 *
 * Envía una cadena de texto a un tópico específico utilizando
 * el nivel de calidad de servicio (QoS) indicado.
 *
 * @param topic Tópico MQTT de destino.
 * @param payload Mensaje a publicar.
 * @param qos Nivel de QoS utilizado en la publicación.
 * @param timeout_ms Tiempo máximo de espera para completar la operación.
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
 * utilizados por el módulo.
 */
void mqtt_manager_stop(void);

/**
 * @brief Consulta el estado actual de la conexión MQTT.
 *
 * @return true si el cliente está conectado al broker.
 * @return false si no existe conexión activa.
 */
bool mqtt_is_connected(void);

#endif /* MQTT_MANAGER_H */