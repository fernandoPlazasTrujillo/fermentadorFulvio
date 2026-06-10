/**
 * @file sd_card.c
 * @brief Implementación del manejo de tarjeta SD mediante SPI.
 * 
 * Este módulo permite montar la tarjeta SD y realizar escrituras
 * seguras en un archivo utilizando protección por mutex.
 * 
 * Incluye control de concurrencia para evitar conflictos en el bus SPI.
 * 
 * @author
 * Fernando Plazas Trujillo
 * Isabella Ordoñez
 * Juan Daniel Constain
 */

#include <stdio.h>
#include <stdbool.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>

#include "esp_log.h"
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"
#include "driver/spi_common.h"
#include "driver/sdspi_host.h"

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

#include "sd_card.h"

static const char *TAG = "SD";

// ============================
// PINES SPI
// ============================

/** @brief Pin MISO */
#define PIN_MISO 19

/** @brief Pin MOSI */
#define PIN_MOSI 23

/** @brief Pin CLK */
#define PIN_CLK  18

/** @brief Pin CS */
#define PIN_CS   5

// ============================
// MONTAJE
// ============================

/** @brief Punto de montaje del sistema de archivos */
static const char mount_point[] = "/sdcard";

/** @brief Archivo de respaldo para mensajes MQTT no enviados */
static const char pending_mqtt_path[] = "/sdcard/mqtt_pending.txt";

// ============================
// MUTEX SPI
// ============================

/** @brief Mutex para acceso seguro al bus SPI */
static SemaphoreHandle_t spi_mutex = NULL;

/** @brief Indica si la SD ya fue montada */
static bool sd_mounted = false;

// ============================
// INIT SD
// ============================

/**
 * @brief Inicializa y monta la tarjeta SD.
 * 
 * Configura el bus SPI, el dispositivo SD y el sistema de archivos.
 * 
 * @return ESP_OK si la tarjeta se monta correctamente.
 */
esp_err_t sd_card_init(void)
{
    esp_err_t ret;

    if (sd_mounted) {
        return ESP_OK;
    }

    if (spi_mutex == NULL) {
        spi_mutex = xSemaphoreCreateMutex();
        if (spi_mutex == NULL) {
            ESP_LOGE(TAG, "Error creando mutex SPI");
            return ESP_FAIL;
        }
    }

    sdmmc_host_t host = SDSPI_HOST_DEFAULT();

    spi_bus_config_t bus_cfg = {
        .mosi_io_num = PIN_MOSI,
        .miso_io_num = PIN_MISO,
        .sclk_io_num = PIN_CLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 4000,
    };

    ret = spi_bus_initialize(host.slot, &bus_cfg, SDSPI_DEFAULT_DMA);
    if (ret != ESP_OK && ret != ESP_ERR_INVALID_STATE) {
        ESP_LOGE(TAG, "Error inicializando SPI");
        return ret;
    }

    sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
    slot_config.gpio_cs = PIN_CS;
    slot_config.host_id = host.slot;

    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = false,
        .max_files = 5,
        .allocation_unit_size = 16 * 1024
    };

    sdmmc_card_t *card;

    ret = esp_vfs_fat_sdspi_mount(mount_point, &host, &slot_config, &mount_config, &card);

    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Error montando SD (%s)", esp_err_to_name(ret));
        return ret;
    }

    ESP_LOGI(TAG, "SD montada correctamente");

    sd_mounted = true;

    return ESP_OK;
}

// ============================
// WRITE
// ============================

/**
 * @brief Escribe una línea en el archivo log.csv de la SD.
 * 
 * La operación es protegida con mutex para evitar accesos concurrentes.
 * 
 * @param line Texto a escribir.
 * @return ESP_OK si la escritura fue exitosa.
 */
esp_err_t sd_card_write_line(const char *line)
{
    if (line == NULL) {
        return ESP_FAIL;
    }

    if (spi_mutex != NULL) {
        xSemaphoreTake(spi_mutex, portMAX_DELAY);
    }

    FILE *f = fopen("/sdcard/log.csv", "a");

    if (f == NULL) {
        ESP_LOGE(TAG, "Error abriendo archivo");
        
        if (spi_mutex != NULL) {
            xSemaphoreGive(spi_mutex);
        }

        return ESP_FAIL;
    }

    fprintf(f, "%s\n", line);

    fflush(f);
    fclose(f);

    if (spi_mutex != NULL) {
        xSemaphoreGive(spi_mutex);
    }

    return ESP_OK;
}

esp_err_t sd_card_append_pending_mqtt(const char *payload)
{
    if (payload == NULL) {
        return ESP_FAIL;
    }

    if (sd_card_init() != ESP_OK) {
        return ESP_FAIL;
    }

    if (spi_mutex != NULL) {
        xSemaphoreTake(spi_mutex, portMAX_DELAY);
    }

    FILE *f = fopen(pending_mqtt_path, "a");

    if (f == NULL) {
        ESP_LOGE(TAG, "Error abriendo pendientes MQTT (%s), errno=%d", pending_mqtt_path, errno);

        if (spi_mutex != NULL) {
            xSemaphoreGive(spi_mutex);
        }

        return ESP_FAIL;
    }

    fprintf(f, "%s\n", payload);
    fflush(f);
    fclose(f);

    if (spi_mutex != NULL) {
        xSemaphoreGive(spi_mutex);
    }

    return ESP_OK;
}

bool sd_card_pending_mqtt_exists(void)
{
    struct stat file_stat;

    if (sd_card_init() != ESP_OK) {
        return false;
    }

    return stat(pending_mqtt_path, &file_stat) == 0;
}

FILE *sd_card_open_pending_mqtt_read(void)
{
    if (sd_card_init() != ESP_OK) {
        return NULL;
    }

    if (spi_mutex != NULL) {
        xSemaphoreTake(spi_mutex, portMAX_DELAY);
    }

    FILE *f = fopen(pending_mqtt_path, "r");

    if (f == NULL) {
        ESP_LOGE(TAG, "Error abriendo pendientes MQTT (%s), errno=%d", pending_mqtt_path, errno);

        if (spi_mutex != NULL) {
            xSemaphoreGive(spi_mutex);
        }
    }

    return f;
}

char *sd_card_read_pending_mqtt_line(FILE *file, char *buffer, size_t size)
{
    if (file == NULL || buffer == NULL || size == 0) {
        return NULL;
    }

    return fgets(buffer, size, file);
}

void sd_card_close_pending_mqtt(FILE *file)
{
    if (file != NULL) {
        fclose(file);
    }

    if (spi_mutex != NULL) {
        xSemaphoreGive(spi_mutex);
    }
}

esp_err_t sd_card_delete_pending_mqtt(void)
{
    if (sd_card_init() != ESP_OK) {
        return ESP_FAIL;
    }

    if (spi_mutex != NULL) {
        xSemaphoreTake(spi_mutex, portMAX_DELAY);
    }

    int result = remove(pending_mqtt_path);

    if (spi_mutex != NULL) {
        xSemaphoreGive(spi_mutex);
    }

    if (result == 0 || errno == ENOENT) {
        return ESP_OK;
    }

    ESP_LOGE(TAG, "Error eliminando pendientes MQTT (%s), errno=%d", pending_mqtt_path, errno);
    return ESP_FAIL;
}

bool sd_card_is_mounted(void)
{
    return sd_mounted;
}