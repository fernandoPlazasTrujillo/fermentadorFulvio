/**
 * @file lcd_parallel.h
 * @brief Driver para pantalla LCD 16x2 en modo paralelo (4 bits).
 *
 * Este módulo proporciona una interfaz sencilla para controlar
 * una pantalla LCD basada en el controlador HD44780 utilizando
 * el bus paralelo en modo de 4 bits.
 *
 * Funcionalidades:
 * - Inicialización del display
 * - Posicionamiento del cursor
 * - Impresión de texto
 * - Limpieza de pantalla
 *
 * @note Este módulo no es thread-safe. Si es accedido desde múltiples
 * tareas, debe protegerse mediante un mutex.
 *
 * @authors
 * - Fernando Plazas
 * - Isabella Ordoñez
 * - Juan Daniel Constain
 */

#pragma once

#include <stdint.h>

/**
 * @brief Inicializa la pantalla LCD
 *
 * Configura los pines GPIO y ejecuta la secuencia de inicialización
 * requerida por el controlador HD44780 en modo 4 bits.
 *
 * @note Debe ejecutarse una sola vez al inicio del sistema.
 */
void lcd_init(void);

/**
 * @brief Limpia la pantalla LCD
 *
 * Borra todo el contenido del display y posiciona el cursor
 * en la posición inicial (0,0).
 */
void lcd_clear(void);

/**
 * @brief Posiciona el cursor en la LCD
 *
 * @param col Columna (0–15)
 * @param row Fila (0–1)
 *
 * @note Internamente convierte a dirección DDRAM.
 */
void lcd_set_cursor(uint8_t col, uint8_t row);

/**
 * @brief Imprime una cadena de caracteres en la LCD
 *
 * @param str Cadena terminada en null
 *
 * @note No realiza control de longitud. Evitar overflow visual.
 */
void lcd_print(const char *str);