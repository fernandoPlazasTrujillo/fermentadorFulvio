/**
 * @file sd_card.c
 * @brief Implementación del manejo de tarjeta SD mediante SPI.
 *
 * Este módulo configura el bus SPI y monta una tarjeta SD usando
 * el sistema de archivos FAT a través de la capa VFS de ESP-IDF.
 *
 * Funcionalidades:
 * - Inicialización del bus SPI
 * - Montaje del sistema de archivos
 * - Escritura de datos en archivo
 *
 * @note El acceso a la SD debe ser protegido mediante mutex en
 * sistemas multitarea (FreeRTOS).
 *
 * @warning Escrituras concurrentes sin protección pueden causar
 * corrupción de datos en la tarjeta.
 *
 * @authors
 * - Fernando Plazas
 * - Isabella Ordoñez
 * - Juan Daniel Constain
 */

#include "sd_card.h"
#include <stdio.h>

#include "esp_err.h"
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"
#include "driver/spi_common.h"

#include <errno.h>
#include <sys/stat.h>

// ==========================
// CONFIGURACIÓN DE PINES SPI
// ==========================

#define PIN_NUM_MISO 19 /**< Pin MISO */
#define PIN_NUM_MOSI 23 /**< Pin MOSI */
#define PIN_NUM_CLK  18 /**< Pin CLK */
#define PIN_NUM_CS   4 /**< Pin Chip Select */
#define SD_SPI_MAX_FREQ_KHZ 4000
#define SD_PENDING_MQTT_PATH "/sdcard/test.txt"

// ==========================
// INICIALIZACIÓN SD
// ==========================

int sd_init(void)
{
    esp_err_t ret;

    /**
     * @brief Configuración del host SPI en modo SD
     */
    sdmmc_host_t host = SDSPI_HOST_DEFAULT();
    host.max_freq_khz = SD_SPI_MAX_FREQ_KHZ;

    /**
     * @brief Configuración del bus SPI
     */
    spi_bus_config_t bus_cfg = {
        .mosi_io_num = PIN_NUM_MOSI,
        .miso_io_num = PIN_NUM_MISO,
        .sclk_io_num = PIN_NUM_CLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 4000
    };

    /**
     * @brief Inicialización del bus SPI
     */
    ret = spi_bus_initialize(host.slot, &bus_cfg, SDSPI_DEFAULT_DMA);
    if (ret != ESP_OK)
    {
        printf("Error inicializando bus SPI\n");
        return -1;
    }

    /**
     * @brief Configuración del dispositivo SD en SPI
     */
    sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
    slot_config.gpio_cs = PIN_NUM_CS;
    slot_config.host_id = host.slot;

    /**
     * @brief Configuración de montaje del sistema de archivos
     */
    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = false,
        .max_files = 5
    };

    sdmmc_card_t *card;

    /**
     * @brief Montaje del sistema de archivos FAT
     */
    ret = esp_vfs_fat_sdspi_mount("/sdcard", &host, &slot_config, &mount_config, &card);

    if (ret != ESP_OK)
    {
        printf("Error montando sistema de archivos en SD: %s\n", esp_err_to_name(ret));
        return -1;
    }

    printf("Tarjeta SD inicializada correctamente\n");
    sdmmc_card_print_info(stdout, card);

    return 0;
}

// ==========================
// ESCRITURA EN ARCHIVO
// ==========================

int sd_write_line(const char *line)
{
    /**
     * @brief Apertura del archivo en modo append
     */
    FILE *f = fopen("/sdcard/log.txt", "a");

    if (f == NULL)
    {
        printf("Error abriendo archivo en SD\n");
        return -1;
    }

    /**
     * @brief Escritura de la línea en el archivo
     */
    fprintf(f, "%s\n", line);

    /**
     * @brief Cierre del archivo para asegurar integridad
     */
    fclose(f);

    return 0;
}

int sd_append_pending_mqtt(const char *payload)
{
    FILE *f = fopen(SD_PENDING_MQTT_PATH, "a");

    if (f == NULL)
    {
        printf("ERROR: No se pudo abrir %s. errno=%d\n", SD_PENDING_MQTT_PATH, errno);
        return -1;
    }

    fprintf(f, "%s\n", payload);

    fclose(f);

    printf("Payload almacenado en %s\n", SD_PENDING_MQTT_PATH);

    return 0;
}

bool sd_pending_mqtt_exists(void)
{
    struct stat file_stat;

    return stat(SD_PENDING_MQTT_PATH, &file_stat) == 0;
}

FILE *sd_open_pending_mqtt_read(void)
{
    FILE *f = fopen(SD_PENDING_MQTT_PATH, "r");

    if (f == NULL)
    {
        printf("ERROR: No se pudo abrir %s en lectura. errno=%d\n", SD_PENDING_MQTT_PATH, errno);
    }

    return f;
}

char *sd_read_pending_mqtt_line(FILE *file, char *buffer, size_t size)
{
    if (file == NULL || buffer == NULL || size == 0)
    {
        return NULL;
    }

    return fgets(buffer, size, file);
}

void sd_close_pending_mqtt(FILE *file)
{
    if (file != NULL)
    {
        fclose(file);
    }
}

int sd_delete_pending_mqtt(void)
{
    if (remove(SD_PENDING_MQTT_PATH) == 0)
    {
        printf("Archivo %s eliminado\n", SD_PENDING_MQTT_PATH);
        return 0;
    }

    if (errno == ENOENT)
    {
        return 0;
    }

    printf("ERROR: No se pudo eliminar %s. errno=%d\n", SD_PENDING_MQTT_PATH, errno);
    return -1;
}
