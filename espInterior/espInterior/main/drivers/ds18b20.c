#include "drivers/ds18b20.h"
#include "driver/gpio.h"
#include "esp_rom_sys.h"
#include "esp_log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "DS18B20";

static gpio_num_t ds_pin;

// =====================
// LOW LEVEL
// =====================
static void ow_low()
{
    gpio_set_direction(ds_pin, GPIO_MODE_OUTPUT);
    gpio_set_level(ds_pin, 0);
}

static void ow_release()
{
    gpio_set_direction(ds_pin, GPIO_MODE_INPUT);
}

static int ow_read()
{
    return gpio_get_level(ds_pin);
}

// =====================
// RESET
// =====================
static int ow_reset()
{
    ow_low();
    esp_rom_delay_us(480);

    ow_release();
    esp_rom_delay_us(70);

    int presence = !ow_read();

    esp_rom_delay_us(410);

    return presence;
}

// =====================
// WRITE BIT
// =====================
static void ow_write_bit(int bit)
{
    ow_low();

    if (bit) {
        esp_rom_delay_us(10);
        ow_release();
        esp_rom_delay_us(55);
    } else {
        esp_rom_delay_us(65);
        ow_release();
        esp_rom_delay_us(5);
    }
}

// =====================
// READ BIT
// =====================
static int ow_read_bit()
{
    int bit;

    ow_low();
    esp_rom_delay_us(3);
    ow_release();
    esp_rom_delay_us(10);

    bit = ow_read();

    esp_rom_delay_us(53);

    return bit;
}

// =====================
// BYTE
// =====================
static void ow_write_byte(uint8_t byte)
{
    for (int i = 0; i < 8; i++) {
        ow_write_bit(byte & 0x01);
        byte >>= 1;
    }
}

static uint8_t ow_read_byte()
{
    uint8_t byte = 0;

    for (int i = 0; i < 8; i++) {
        byte |= (ow_read_bit() << i);
    }

    return byte;
}

// =====================
// INIT
// =====================
esp_err_t ds18b20_init(gpio_num_t pin)
{
    ds_pin = pin;

    gpio_set_pull_mode(ds_pin, GPIO_PULLUP_ONLY);

    ESP_LOGI(TAG, "DS18B20 init en GPIO %d", pin);

    return ESP_OK;
}

// =====================
// READ TEMPERATURE
// =====================
float ds18b20_read_temperature(void)
{
    if (!ow_reset()) {
        ESP_LOGE(TAG, "Sensor no detectado");
        return -127.0;
    }

    // Skip ROM
    ow_write_byte(0xCC);

    // Convert T
    ow_write_byte(0x44);

    // Espera conversión (750ms max)
    vTaskDelay(pdMS_TO_TICKS(750));

    if (!ow_reset()) {
        return -127.0;
    }

    ow_write_byte(0xCC);
    ow_write_byte(0xBE);

    uint8_t temp_lsb = ow_read_byte();
    uint8_t temp_msb = ow_read_byte();

    int16_t raw = (temp_msb << 8) | temp_lsb;

    float temp = raw / 16.0;

    return temp;
}