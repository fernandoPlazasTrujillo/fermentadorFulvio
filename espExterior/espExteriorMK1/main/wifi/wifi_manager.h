#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <stdbool.h>
#include <stdint.h>

/**
 * @brief Inicializa y conecta el ESP32 al WiFi.
 */
void wifi_init(void);

/**
 * @brief Espera hasta que el WiFi obtenga IP o se cumpla el timeout.
 *
 * @param timeout_ms Tiempo maximo de espera en milisegundos.
 * @return true si el WiFi se conecto, false si expiro el timeout.
 */
bool wifi_wait_connected(uint32_t timeout_ms);

/**
 * @brief Escanea redes WiFi visibles y las imprime por consola.
 */
void wifi_scan_print(void);

#endif
