/**
 * @file wifi_manager.h
 * @brief Gestión de conectividad WiFi para el ESP32.
 *
 * Este módulo proporciona una interfaz para la inicialización,
 * conexión y monitoreo de la red WiFi utilizada por el sistema.
 *
 * Responsabilidades:
 * - Inicializar la pila TCP/IP.
 * - Configurar la interfaz WiFi.
 * - Gestionar la conexión a la red inalámbrica.
 * - Verificar el estado de conexión.
 * - Realizar escaneo de redes disponibles.
 *
 * Este módulo abstrae los detalles de la implementación WiFi,
 * permitiendo que otras tareas del sistema utilicen servicios
 * de red sin depender directamente de ESP-IDF.
 *
 * @authors
 * - Fernando Plazas
 * - Isabella Ordoñez
 * - Juan Daniel Constain
 */

#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <stdbool.h>
#include <stdint.h>

/**
 * @brief Inicializa y conecta el ESP32 a una red WiFi.
 *
 * Configura los recursos necesarios de red e inicia
 * el proceso de asociación con el punto de acceso.
 */
void wifi_init(void);

/**
 * @brief Espera hasta que se establezca la conexión WiFi.
 *
 * La función bloquea hasta que el dispositivo obtenga una
 * dirección IP válida o se alcance el tiempo máximo de espera.
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
 * Realiza un barrido de redes inalámbricas visibles e imprime
 * la información obtenida mediante la consola de depuración.
 */
void wifi_scan_print(void);

#endif