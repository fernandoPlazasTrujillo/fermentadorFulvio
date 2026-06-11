/**
 * @file lcd_parallel.h
 * @brief Driver para pantalla LCD 16x2 en modo paralelo (4 bits).
 *
 * Este módulo proporciona una interfaz para controlar una pantalla
 * LCD basada en el controlador HD44780 utilizando comunicación
 * paralela de 4 bits.
 *
 * Funcionalidades principales:
 * - Inicialización del display.
 * - Posicionamiento del cursor.
 * - Impresión de texto.
 * - Limpieza de pantalla.
 *
 * @section lcd_architecture Integración en el sistema
 *
 * Dentro del sistema distribuido de fermentación de café, esta pantalla
 * forma parte del ESP32 externo y permite visualizar información de:
 *
 * - Temperatura ambiente.
 * - Humedad relativa.
 * - Estado de conexión WiFi.
 * - Estado de comunicación MQTT.
 * - Estado de almacenamiento en SD.
 * - Información de diagnóstico.
 *
 * @note Este módulo no es thread-safe. Si es utilizado desde múltiples
 * tareas FreeRTOS debe protegerse mediante un mutex.
 *
 * @authors
 * - Fernando Plazas
 * - Isabella Ordoñez
 * - Juan Daniel Constain
 */

#pragma once

#include <stdint.h>

/**
 * @brief Inicializa la pantalla LCD.
 *
 * Configura los GPIO asociados a las líneas de datos y control,
 * ejecutando posteriormente la secuencia de inicialización definida
 * por el controlador HD44780 para operación en modo de 4 bits.
 *
 * Esta función debe ejecutarse antes de utilizar cualquier otra
 * función del módulo.
 *
 * @note Debe llamarse una única vez durante la fase de arranque
 * del sistema.
 */
void lcd_init(void);

/**
 * @brief Limpia completamente el contenido de la pantalla.
 *
 * Borra todos los caracteres visibles y reposiciona el cursor
 * en la posición inicial de la primera fila.
 *
 * @note Esta operación requiere varios milisegundos debido a las
 * limitaciones temporales del controlador HD44780.
 */
void lcd_clear(void);

/**
 * @brief Posiciona el cursor en una ubicación específica.
 *
 * Convierte automáticamente la fila y columna indicadas a la
 * dirección DDRAM correspondiente del controlador LCD.
 *
 * @param col Columna destino.
 *            Rango válido: 0–15.
 *
 * @param row Fila destino.
 *            Rango válido: 0–1.
 *
 * @note Valores fuera de rango pueden producir comportamientos
 * inesperados dependiendo de la implementación.
 */
void lcd_set_cursor(uint8_t col, uint8_t row);

/**
 * @brief Imprime una cadena de caracteres en la pantalla.
 *
 * Los caracteres son enviados secuencialmente al controlador LCD
 * comenzando desde la posición actual del cursor.
 *
 * @param str Cadena de caracteres terminada en '\0'.
 *
 * @note No se realiza control automático de longitud ni salto
 * de línea. Es responsabilidad de la aplicación garantizar que
 * el texto se ajuste al tamaño disponible del display.
 */
void lcd_print(const char *str);