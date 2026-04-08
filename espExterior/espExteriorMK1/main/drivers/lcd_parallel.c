/**
 * @file lcd_parallel.c
 * @brief Implementación del driver para LCD 16x2 en modo paralelo (4 bits).
 *
 * Este módulo implementa el control de una pantalla LCD basada en
 * el controlador HD44780 utilizando GPIO del ESP32.
 *
 * Características:
 * - Comunicación en modo 4 bits
 * - Control manual de temporización
 * - Soporte para comandos y datos
 *
 * @note No es seguro para acceso concurrente.
 * @warning Requiere temporización precisa (uso de delays en microsegundos)
 *
 * @authors
 * - Fernando Plazas
 * - Isabella Ordoñez
 * - Juan Daniel Constain
 */

#include "lcd_parallel.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_rom_sys.h"

// ==========================
// CONFIGURACIÓN DE PINES
// ==========================

#define LCD_RS GPIO_NUM_13 /**< Pin Register Select */
#define LCD_E  GPIO_NUM_33 /**< Pin Enable */

#define LCD_D4 GPIO_NUM_14 /**< Data bit 4 */
#define LCD_D5 GPIO_NUM_27 /**< Data bit 5 */
#define LCD_D6 GPIO_NUM_26 /**< Data bit 6 */
#define LCD_D7 GPIO_NUM_25 /**< Data bit 7 */

// ==========================
// FUNCIONES INTERNAS
// ==========================

/**
 * @brief Envía un nibble (4 bits) al LCD
 *
 * Esta función coloca los bits en las líneas D4-D7 y genera
 * un pulso en el pin Enable para registrar el dato.
 *
 * @param nibble Valor de 4 bits a enviar (LSB alineado)
 *
 * @note Función de bajo nivel crítica para la comunicación
 */
static void lcd_send_nibble(uint8_t nibble)
{
    gpio_set_level(LCD_D4, (nibble >> 0) & 1);
    gpio_set_level(LCD_D5, (nibble >> 1) & 1);
    gpio_set_level(LCD_D6, (nibble >> 2) & 1);
    gpio_set_level(LCD_D7, (nibble >> 3) & 1);

    gpio_set_level(LCD_E, 1);
    esp_rom_delay_us(1);
    gpio_set_level(LCD_E, 0);
    esp_rom_delay_us(100);
}

/**
 * @brief Envía un byte completo al LCD
 *
 * Divide el byte en dos nibbles (alto y bajo) y los envía
 * secuencialmente usando lcd_send_nibble.
 *
 * @param data Byte a enviar
 * @param rs Selección de registro:
 * - 0: comando
 * - 1: dato
 */
static void lcd_send_byte(uint8_t data, int rs)
{
    gpio_set_level(LCD_RS, rs);

    lcd_send_nibble(data >> 4);
    lcd_send_nibble(data & 0x0F);
}

// ==========================
// FUNCIONES DE COMANDO
// ==========================

/**
 * @brief Envía un comando al LCD
 *
 * @param cmd Código de comando según HD44780
 */
static void lcd_cmd(uint8_t cmd)
{
    lcd_send_byte(cmd, 0);
}

/**
 * @brief Envía un dato (carácter) al LCD
 *
 * @param data Caracter ASCII a mostrar
 */
static void lcd_data(uint8_t data)
{
    lcd_send_byte(data, 1);
}

// ==========================
// API PÚBLICA
// ==========================

void lcd_init(void)
{
    // Configuración de pines como salida
    gpio_set_direction(LCD_RS, GPIO_MODE_OUTPUT);
    gpio_set_direction(LCD_E,  GPIO_MODE_OUTPUT);

    gpio_set_direction(LCD_D4, GPIO_MODE_OUTPUT);
    gpio_set_direction(LCD_D5, GPIO_MODE_OUTPUT);
    gpio_set_direction(LCD_D6, GPIO_MODE_OUTPUT);
    gpio_set_direction(LCD_D7, GPIO_MODE_OUTPUT);

    // Espera inicial para estabilización del display
    vTaskDelay(pdMS_TO_TICKS(50));

    // Secuencia de inicialización estándar (HD44780)
    lcd_send_nibble(0x03);
    vTaskDelay(pdMS_TO_TICKS(5));

    lcd_send_nibble(0x03);
    vTaskDelay(pdMS_TO_TICKS(5));

    lcd_send_nibble(0x03);
    vTaskDelay(pdMS_TO_TICKS(5));

    lcd_send_nibble(0x02); // Activar modo 4 bits

    // Configuración del display
    lcd_cmd(0x28); // 2 líneas, matriz 5x8
    lcd_cmd(0x0C); // Display ON, cursor OFF
    lcd_cmd(0x06); // Incremento automático del cursor
    lcd_cmd(0x01); // Limpiar pantalla

    vTaskDelay(pdMS_TO_TICKS(5));
}

void lcd_clear(void)
{
    lcd_cmd(0x01);
    vTaskDelay(pdMS_TO_TICKS(5));
}

void lcd_set_cursor(uint8_t col, uint8_t row)
{
    uint8_t addr = (row == 0) ? 0x80 : 0xC0;
    lcd_cmd(addr + col);
}

void lcd_print(const char *str)
{
    while (*str)
    {
        lcd_data(*str++);
    }
}