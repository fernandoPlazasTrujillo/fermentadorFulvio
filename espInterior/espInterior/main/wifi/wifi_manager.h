/**
 * @file wifi_manager.h
 * @brief Interfaz para la gestión de conectividad WiFi.
 *
 * Este módulo proporciona las funciones necesarias para
 * inicializar la interfaz WiFi del ESP32, establecer conexión
 * con una red inalámbrica y consultar el estado de conectividad.
 *
 * Funcionalidades:
 * - Inicialización del subsistema WiFi.
 * - Conexión automática a una red configurada.
 * - Espera sincronizada de conexión.
 * - Escaneo de redes disponibles.
 * - Consulta del estado de conexión.
 *
 * El módulo abstrae la complejidad de la configuración WiFi
 * del ESP-IDF y ofrece una interfaz simplificada para las
 * tareas de aplicación.
 *
 * @author
 * Fernando Plazas Trujillo
 * Isabella Ordoñez
 * Juan Daniel Constain
 */

#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <stdbool.h>
#include <stdint.h>

/**
 * @brief Inicializa la interfaz WiFi y establece la conexión.
 *
 * Configura el subsistema WiFi del ESP32, registra los eventos
 * necesarios y comienza el proceso de conexión a la red
 * previamente configurada.
 */
void wifi_init(void);

/**
 * @brief Espera hasta que se establezca una conexión WiFi.
 *
 * La función bloquea la ejecución hasta que el dispositivo
 * obtenga una dirección IP válida o hasta que expire el
 * tiempo máximo de espera especificado.
 *
 * @param timeout_ms Tiempo máximo de espera en milisegundos.
 *
 * @return true si la conexión fue exitosa.
 * @return false si ocurrió timeout o error.
 */
bool wifi_wait_connected(uint32_t timeout_ms);

/**
 * @brief Escanea las redes WiFi disponibles.
 *
 * Realiza un escaneo activo del entorno inalámbrico e imprime
 * los resultados mediante la consola de depuración.
 */
void wifi_scan_print(void);

/**
 * @brief Consulta el estado actual de la conexión WiFi.
 *
 * @return true si el ESP32 está conectado a una red WiFi.
 * @return false si no existe conexión activa.
 */
bool wifi_is_connected(void);

#endif /* WIFI_MANAGER_H */