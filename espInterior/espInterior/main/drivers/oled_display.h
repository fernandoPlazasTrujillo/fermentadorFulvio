#ifndef OLED_DISPLAY_H
#define OLED_DISPLAY_H

#include "esp_err.h"
#include <stdint.h>

esp_err_t oled_init(void);

esp_err_t oled_clear(void);

esp_err_t oled_update(void);

esp_err_t oled_draw_pixel(uint8_t x, uint8_t y);

esp_err_t oled_draw_char(uint8_t x, uint8_t y, char c);

esp_err_t oled_draw_string(uint8_t x, uint8_t y, const char *str);

#endif