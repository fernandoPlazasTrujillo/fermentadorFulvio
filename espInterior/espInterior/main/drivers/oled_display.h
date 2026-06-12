/**
 * @file oled_display.h
 * @brief Driver para pantalla OLED SSD1306 mediante interfaz I2C.
 *
 * Este módulo proporciona funciones para controlar una pantalla
 * OLED basada en el controlador SSD1306.
 *
 * Funcionalidades principales:
 * - Inicialización de la pantalla.
 * - Limpieza del framebuffer.
 * - Actualización de la pantalla.
 * - Dibujo de píxeles.
 * - Renderizado de caracteres.
 * - Renderizado de cadenas de texto.
 *
 * La pantalla es utilizada para visualizar información del proceso
 * de fermentación, incluyendo variables ambientales, estado de los
 * actuadores y eventos del sistema.
 *
 * @author
 * Fernando Plazas Trujillo
 * Isabella Ordoñez
 * Juan Daniel Constain
 */
#ifndef OLED_DISPLAY_H
#define OLED_DISPLAY_H

#include "esp_err.h"
#include <stdint.h>

/**
 * @brief Inicializa la pantalla OLED.
 *
 * Configura el controlador SSD1306 y prepara el framebuffer
 * para operaciones de dibujo.
 *
 * Esta función debe ejecutarse una única vez durante el
 * arranque del sistema.
 *
 * @return
 * - ESP_OK si la inicialización fue exitosa.
 * - Código de error en caso contrario.
 */
esp_err_t oled_init(void);

/**
 * @brief Borra el contenido del framebuffer.
 *
 * Establece todos los píxeles en estado apagado.
 *
 * Es necesario llamar posteriormente a oled_update()
 * para reflejar los cambios en la pantalla física.
 *
 * @return
 * - ESP_OK si la operación fue exitosa.
 * - Código de error en caso contrario.
 */
esp_err_t oled_clear(void);

/**
 * @brief Actualiza la pantalla OLED.
 *
 * Transfiere el contenido actual del framebuffer
 * hacia la pantalla mediante el bus I2C.
 *
 * Debe ejecutarse después de realizar operaciones
 * de dibujo para que los cambios sean visibles.
 *
 * @return
 * - ESP_OK si la actualización fue exitosa.
 * - Código de error en caso contrario.
 */
esp_err_t oled_update(void);

/**
 * @brief Dibuja un píxel en el framebuffer.
 *
 * Activa el píxel correspondiente a las coordenadas
 * especificadas.
 *
 * @param x Coordenada horizontal.
 * @param y Coordenada vertical.
 *
 * @return
 * - ESP_OK si el píxel fue dibujado correctamente.
 * - ESP_ERR_INVALID_ARG si las coordenadas están fuera de rango.
 */
esp_err_t oled_draw_pixel(uint8_t x, uint8_t y);

/**
 * @brief Dibuja un carácter ASCII en el framebuffer.
 *
 * Utiliza la fuente bitmap definida en font5x7.h para
 * representar el carácter solicitado.
 *
 * @param x Coordenada horizontal inicial.
 * @param y Coordenada vertical inicial.
 * @param c Carácter ASCII a representar.
 *
 * @return
 * - ESP_OK si la operación fue exitosa.
 * - Código de error en caso contrario.
 */
esp_err_t oled_draw_char(uint8_t x, uint8_t y, char c);

/**
 * @brief Dibuja una cadena de texto en el framebuffer.
 *
 * Renderiza secuencialmente cada carácter utilizando
 * la fuente configurada por el controlador.
 *
 * @param x Coordenada horizontal inicial.
 * @param y Coordenada vertical inicial.
 * @param str Cadena de caracteres terminada en NULL.
 *
 * @return
 * - ESP_OK si la operación fue exitosa.
 * - Código de error en caso contrario.
 */
esp_err_t oled_draw_string(uint8_t x, uint8_t y, const char *str);

#endif