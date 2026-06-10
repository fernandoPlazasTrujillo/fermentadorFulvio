#include "drivers/oled_display.h"

#include <string.h>

#include "hal/i2c_manager.h"
#include "freertos/semphr.h"

#include "font5x7.h"

#define OLED_ADDR      0x3C

#define OLED_WIDTH     128
#define OLED_HEIGHT    64
#define OLED_PAGES     8

extern SemaphoreHandle_t mutex_i2c;

static uint8_t framebuffer[1024];

static esp_err_t oled_send_command(uint8_t cmd)
{
    uint8_t buffer[2];

    buffer[0] = 0x00;
    buffer[1] = cmd;

    if (xSemaphoreTake(mutex_i2c, pdMS_TO_TICKS(100)))
    {
        esp_err_t ret =
            i2c_manager_write_raw(
                OLED_ADDR,
                buffer,
                sizeof(buffer));

        xSemaphoreGive(mutex_i2c);

        return ret;
    }

    return ESP_FAIL;
}

static esp_err_t oled_send_data(const uint8_t *data, size_t len) 
{
    uint8_t buffer[129];

    buffer[0] = 0x40;

    memcpy(&buffer[1], data, len);

    if (xSemaphoreTake(mutex_i2c, pdMS_TO_TICKS(100)))
    {
        esp_err_t ret =
            i2c_manager_write_raw(
                OLED_ADDR,
                buffer,
                len + 1);

        xSemaphoreGive(mutex_i2c);

        return ret;
    }

    return ESP_FAIL;
}

esp_err_t oled_init(void)
{
    oled_send_command(0xAE);

    oled_send_command(0x20);
    oled_send_command(0x00);

    oled_send_command(0xB0);

    oled_send_command(0xC8);

    oled_send_command(0x00);
    oled_send_command(0x10);

    oled_send_command(0x40);

    oled_send_command(0x81);
    oled_send_command(0xFF);

    oled_send_command(0xA1);

    oled_send_command(0xA6);

    oled_send_command(0xA8);
    oled_send_command(0x3F);

    oled_send_command(0xA4);

    oled_send_command(0xD3);
    oled_send_command(0x00);

    oled_send_command(0xD5);
    oled_send_command(0xF0);

    oled_send_command(0xD9);
    oled_send_command(0x22);

    oled_send_command(0xDA);
    oled_send_command(0x12);

    oled_send_command(0xDB);
    oled_send_command(0x20);

    oled_send_command(0x8D);
    oled_send_command(0x14);

    oled_send_command(0xAF);

    return ESP_OK;
}

esp_err_t oled_clear(void)
{
    memset(
        framebuffer,
        0,
        sizeof(framebuffer));

    return oled_update();
}

esp_err_t oled_draw_pixel(uint8_t x, uint8_t y)
{
    if (x >= OLED_WIDTH)
        return ESP_FAIL;

    if (y >= OLED_HEIGHT)
        return ESP_FAIL;

    framebuffer[
        x + (y / 8) * OLED_WIDTH
    ] |= (1 << (y % 8));

    return ESP_OK;
}

esp_err_t oled_draw_char(uint8_t x, uint8_t y, char c)
{
    if ((uint8_t)c >= sizeof(font5x7)/5)
        return ESP_FAIL;

    for (int col = 0; col < 5; col++)
    {
        uint8_t line = font5x7[(uint8_t)c][col];

        for (int row = 0; row < 7; row++)
        {
            if (line & (1 << row))
            {
                oled_draw_pixel(
                    x + col,
                    y + row);
            }
        }
    }

    return ESP_OK;
}

esp_err_t oled_draw_string(
    uint8_t x,
    uint8_t y,
    const char *str)
{
    while (*str)
    {
        oled_draw_char(
            x,
            y,
            *str);

        x += 6;
        str++;
    }

    return ESP_OK;
}

esp_err_t oled_update(void)
{
    for (int page = 0;
         page < OLED_PAGES;
         page++)
    {
        oled_send_command(
            0xB0 + page);

        oled_send_command(0x00);
        oled_send_command(0x10);

        oled_send_data(
            &framebuffer[
                page * OLED_WIDTH],
            OLED_WIDTH);
    }

    return ESP_OK;
}

