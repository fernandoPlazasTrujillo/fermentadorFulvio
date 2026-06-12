/**
 * @file oled_display.c
 * @brief Implementación del controlador OLED SSD1306 mediante interfaz I2C.
 *
 * Este módulo implementa las funciones necesarias para controlar una
 * pantalla OLED basada en el controlador SSD1306.
 *
 * Funcionalidades:
 * - Inicialización del controlador.
 * - Gestión de framebuffer local.
 * - Dibujo de píxeles.
 * - Renderizado de caracteres.
 * - Renderizado de cadenas de texto.
 * - Actualización de pantalla mediante I2C.
 *
 * El acceso al bus I2C es protegido mediante un mutex compartido
 * para garantizar la exclusión mutua entre tareas FreeRTOS.
 *
 * @author
 * Fernando Plazas Trujillo
 * Isabella Ordoñez
 * Juan Daniel Constain
 */
#include "drivers/oled_display.h"

#include <string.h>

#include "hal/i2c_manager.h"
#include "freertos/semphr.h"

#include "font5x7.h"

/**
 * @brief Dirección I2C del controlador SSD1306.
 */
#define OLED_ADDR      0x3C

/**
 * @brief Ancho de la pantalla OLED en píxeles.
 */
#define OLED_WIDTH 128

/**
 * @brief Alto de la pantalla OLED en píxeles.
 */
#define OLED_HEIGHT 64

/**
 * @brief Número de páginas de memoria del SSD1306.
 *
 * Cada página contiene 8 filas de píxeles.
 */
#define OLED_PAGES 8

/**
 * @brief Mutex compartido para acceso exclusivo al bus I2C.
 *
 * Utilizado por todos los periféricos que comparten el mismo bus,
 * evitando condiciones de carrera entre tareas concurrentes.
 */
extern SemaphoreHandle_t mutex_i2c;

/**
 * @brief Framebuffer local de la pantalla OLED.
 *
 * Almacena una copia completa de la memoria gráfica del SSD1306.
 * Los cambios gráficos se realizan sobre este buffer y posteriormente
 * se transfieren al dispositivo mediante oled_update().
 */
static uint8_t framebuffer[1024];

/**
 * @brief Envía un comando al controlador SSD1306.
 *
 * La transmisión se realiza mediante I2C utilizando exclusión
 * mutua sobre el bus compartido.
 *
 * @param cmd Comando a transmitir.
 *
 * @return
 * - ESP_OK si la transmisión fue exitosa.
 * - ESP_FAIL si no fue posible acceder al bus.
 */
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

/**
 * @brief Envía datos gráficos al SSD1306.
 *
 * Copia los datos al buffer de transmisión I2C y posteriormente
 * los envía al controlador OLED.
 *
 * @param data Puntero al bloque de datos.
 * @param len Cantidad de bytes a transmitir.
 *
 * @return
 * - ESP_OK si la transmisión fue exitosa.
 * - ESP_FAIL en caso de error.
 */
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

/*
 * Secuencia de inicialización recomendada por el datasheet
 * del controlador SSD1306.
 */
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

/*
 * Conversión de coordenadas XY a posición dentro
 * de la memoria organizada por páginas del SSD1306.
 */
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

