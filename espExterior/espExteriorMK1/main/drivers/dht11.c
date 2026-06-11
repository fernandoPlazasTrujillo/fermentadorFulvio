/**
 * @file dht11.c
 * @brief Implementación del driver para sensor DHT11.
 *
 * Este módulo implementa la comunicación de bajo nivel con el sensor DHT11
 * utilizando el protocolo propietario basado en temporización de pulsos.
 *
 * El controlador realiza:
 * - Generación de la señal de inicio.
 * - Recepción de los 40 bits transmitidos por el sensor.
 * - Verificación de integridad mediante checksum.
 * - Conversión de los datos recibidos a temperatura y humedad.
 *
 * @note El protocolo del DHT11 depende de retardos en microsegundos.
 * @note Se recomienda evitar interrupciones prolongadas durante la lectura.
 * @warning El acceso concurrente al sensor debe protegerse mediante mutex
 * cuando sea utilizado desde múltiples tareas FreeRTOS.
 *
 * @authors
 * - Fernando Plazas
 * - Isabella Ordoñez
 * - Juan Daniel Constain
 */

#include "dht11.h"
#include "driver/gpio.h"
#include "esp_rom_sys.h"

/**
 * @brief GPIO utilizado para la comunicación con el sensor DHT11.
 */
#define DHT11_GPIO GPIO_NUM_5

/**
 * @brief Configura el pin de datos como salida.
 *
 * Esta función se utiliza durante la fase de inicio de comunicación,
 * cuando el ESP32 debe enviar la señal de arranque al sensor.
 */
static void set_output()
{
    gpio_set_direction(DHT11_GPIO, GPIO_MODE_OUTPUT);
}

/**
 * @brief Configura el pin de datos como entrada.
 *
 * Esta función se utiliza después de la señal de inicio para permitir
 * que el sensor transmita los datos hacia el ESP32.
 */
static void set_input()
{
    gpio_set_direction(DHT11_GPIO, GPIO_MODE_INPUT);
}

/**
 * @brief Realiza una lectura completa del sensor DHT11.
 *
 * Ejecuta la secuencia completa de comunicación:
 * 1. Envía la señal de inicio al sensor.
 * 2. Espera la respuesta de presencia.
 * 3. Lee los 40 bits de información transmitidos.
 * 4. Verifica la integridad mediante checksum.
 * 5. Extrae los valores de temperatura y humedad.
 *
 * Los datos obtenidos se almacenan en la estructura proporcionada
 * por el usuario.
 *
 * @param[out] data Estructura donde se almacenarán los valores leídos.
 *
 * @retval 0 Lectura realizada correctamente.
 * @retval -1 No se recibió respuesta del sensor.
 * @retval -2 Error de checksum en los datos recibidos.
 *
 * @note Esta función es bloqueante debido al uso de retardos de
 * microsegundos requeridos por el protocolo DHT11.
 */
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
    if (gpio_get_level(DHT11_GPIO) == 1)
        return -1;

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