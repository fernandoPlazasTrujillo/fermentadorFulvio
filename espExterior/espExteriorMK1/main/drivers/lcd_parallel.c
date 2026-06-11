/**
 * @file lcd_parallel.c
 * @brief Implementación del driver para pantalla LCD 16x2 basada en HD44780.
 *
 * Este módulo implementa la comunicación paralela en modo de 4 bits
 * utilizando GPIO del ESP32 para controlar una pantalla LCD 16x2.
 *
 * Funcionalidades implementadas:
 * - Inicialización del controlador HD44780.
 * - Envío de comandos.
 * - Envío de caracteres.
 * - Posicionamiento del cursor.
 * - Limpieza de pantalla.
 *
 * @section lcd_system_role Rol dentro del sistema
 *
 * Este módulo forma parte del ESP32 externo y proporciona la interfaz
 * visual local para el monitoreo del proceso de fermentación.
 *
 * La información mostrada puede incluir:
 * - Temperatura ambiente.
 * - Humedad relativa.
 * - Estado de conexión WiFi.
 * - Estado MQTT.
 * - Estado del almacenamiento SD.
 * - Mensajes de diagnóstico.
 *
 * @note No es seguro para acceso concurrente.
 * @note Se recomienda que únicamente la tarea task_display acceda
 * directamente a este módulo.
 *
 * @warning La comunicación requiere retardos precisos para cumplir
 * las especificaciones del controlador HD44780.
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
#define LCD_D5 GPIO_NUM_32 /**< Data bit 5 */
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

    esp_rom_delay_us(2);
    gpio_set_level(LCD_E, 1);
    esp_rom_delay_us(5);
    gpio_set_level(LCD_E, 0);
    esp_rom_delay_us(120);
}

/**
 * @brief Envía un byte completo al controlador LCD.
 *
 * Debido al uso del modo de comunicación de 4 bits, el byte es
 * dividido en dos nibbles que son transmitidos secuencialmente.
 *
 * @param data Byte a transmitir.
 *
 * @param rs Selección de registro:
 * - 0: Comando.
 * - 1: Dato.
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
 * @brief Envía un comando al controlador HD44780.
 *
 * Permite ejecutar operaciones internas como:
 * - Limpiar pantalla.
 * - Posicionar cursor.
 * - Configurar modos de operación.
 *
 * @param cmd Código de comando HD44780.
 */
static void lcd_cmd(uint8_t cmd)
{
    lcd_send_byte(cmd, 0);
}

/**
 * @brief Envía un carácter al display.
 *
 * El dato es almacenado en la memoria DDRAM interna del
 * controlador y mostrado en la posición actual del cursor.
 *
 * @param data Código ASCII del carácter a visualizar.
 */
static void lcd_data(uint8_t data)
{
    lcd_send_byte(data, 1);
}

/**
 * @brief Inicializa la pantalla LCD.
 *
 * Configura los GPIO asociados a las líneas de control y datos,
 * ejecuta la secuencia de arranque recomendada por el fabricante
 * y habilita el funcionamiento en modo de 4 bits.
 *
 * La secuencia implementada sigue las recomendaciones del
 * controlador HD44780.
 *
 * @note Debe ejecutarse una única vez durante el arranque
 * del sistema.
 */
void lcd_init(void)
{
    // Configuración de pines como salida
    gpio_set_direction(LCD_RS, GPIO_MODE_OUTPUT);
    gpio_set_direction(LCD_E,  GPIO_MODE_OUTPUT);

    gpio_set_direction(LCD_D4, GPIO_MODE_OUTPUT);
    gpio_set_direction(LCD_D5, GPIO_MODE_OUTPUT);
    gpio_set_direction(LCD_D6, GPIO_MODE_OUTPUT);
    gpio_set_direction(LCD_D7, GPIO_MODE_OUTPUT);

    gpio_set_level(LCD_RS, 0);
    gpio_set_level(LCD_E, 0);
    gpio_set_level(LCD_D4, 0);
    gpio_set_level(LCD_D5, 0);
    gpio_set_level(LCD_D6, 0);
    gpio_set_level(LCD_D7, 0);

    // Espera inicial para estabilización del display
    vTaskDelay(pdMS_TO_TICKS(100));

    // Secuencia de inicialización estándar (HD44780)
    lcd_send_nibble(0x03);
    vTaskDelay(pdMS_TO_TICKS(5));

    lcd_send_nibble(0x03);
    vTaskDelay(pdMS_TO_TICKS(5));

    lcd_send_nibble(0x03);
    vTaskDelay(pdMS_TO_TICKS(5));

    lcd_send_nibble(0x02); // Activar modo 4 bits
    vTaskDelay(pdMS_TO_TICKS(5));

    // Configuración del display
    lcd_cmd(0x28); // 2 líneas, matriz 5x8
    lcd_cmd(0x0C); // Display ON, cursor OFF
    lcd_cmd(0x06); // Incremento automático del cursor
    lcd_cmd(0x01); // Limpiar pantalla

    vTaskDelay(pdMS_TO_TICKS(5));
}

/**
 * @brief Limpia completamente el contenido de la pantalla.
 *
 * Borra todos los caracteres visibles y reposiciona el cursor
 * en la primera columna de la primera fila.
 */
void lcd_clear(void)
{
    lcd_cmd(0x01);
    vTaskDelay(pdMS_TO_TICKS(5));
}

/**
 * @brief Posiciona el cursor en una ubicación específica.
 *
 * Convierte la fila y columna solicitadas a la dirección DDRAM
 * correspondiente del controlador HD44780.
 *
 * @param col Columna destino (0-15).
 * @param row Fila destino (0-1).
 */
void lcd_set_cursor(uint8_t col, uint8_t row)
{
    uint8_t addr = (row == 0) ? 0x80 : 0xC0;
    lcd_cmd(addr + col);
}

/**
 * @brief Imprime una cadena de texto en la pantalla.
 *
 * Los caracteres son enviados secuencialmente comenzando desde
 * la posición actual del cursor.
 *
 * @param str Cadena terminada en carácter nulo ('\0').
 *
 * @warning No se realiza control automático de longitud ni
 * salto de línea.
 */
void lcd_print(const char *str)
{
    while (*str)
    {
        lcd_data(*str++);
    }
}
