/**
 * @file dht11.c
 * @brief Implementación del driver para sensor DHT11.
 *
 * Este módulo implementa la comunicación de bajo nivel con el sensor DHT11,
 * basada en tiempos de pulso para la transmisión de datos.
 *
 * @note Este driver es sensible a interrupciones y temporización.
 * @warning No debe ejecutarse en paralelo sin control de CPU.
 *
 * @authors
 * - Fernando Plazas
 * - Isabella Ordoñez
 * - Juan Daniel Constain
 */

#include "dht11.h"
#include "driver/gpio.h"
#include "esp_rom_sys.h"

#define DHT11_GPIO GPIO_NUM_5

/**
 * @brief Configura el pin como salida
 */
static void set_output()
{
    gpio_set_direction(DHT11_GPIO, GPIO_MODE_OUTPUT);
}

/**
 * @brief Configura el pin como entrada
 */
static void set_input()
{
    gpio_set_direction(DHT11_GPIO, GPIO_MODE_INPUT);
}

int dht11_read(dht11_data_t *data)
{
    uint8_t bits[5] = {0};

    // Señal de inicio
    set_output();
    gpio_set_level(DHT11_GPIO, 0);
    esp_rom_delay_us(18000);
    gpio_set_level(DHT11_GPIO, 1);
    esp_rom_delay_us(30);
    set_input();

    // Esperar respuesta del sensor
    if (gpio_get_level(DHT11_GPIO) == 1) return -1;

    while (gpio_get_level(DHT11_GPIO) == 0);
    while (gpio_get_level(DHT11_GPIO) == 1);

    // Leer 40 bits
    for (int i = 0; i < 40; i++)
    {
        while (gpio_get_level(DHT11_GPIO) == 0);

        int t = 0;
        while (gpio_get_level(DHT11_GPIO) == 1)
        {
            esp_rom_delay_us(1);
            t++;
        }

        bits[i / 8] <<= 1;

        if (t > 40)
            bits[i / 8] |= 1;
    }

    // Verificar checksum
    if ((bits[0] + bits[1] + bits[2] + bits[3]) != bits[4])
        return -2;

    data->humedad = bits[0];
    data->temperatura = bits[2];

    return 0;
}