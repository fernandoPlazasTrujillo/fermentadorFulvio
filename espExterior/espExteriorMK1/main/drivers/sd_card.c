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

#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"
#include "driver/spi_common.h"

// ==========================
// CONFIGURACIÓN DE PINES SPI
// ==========================

#define PIN_NUM_MISO 19 /**< Pin MISO */
#define PIN_NUM_MOSI 23 /**< Pin MOSI */
#define PIN_NUM_CLK  18 /**< Pin CLK */
#define PIN_NUM_CS   15 /**< Pin Chip Select */

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

    /**
     * @brief Configuración del bus SPI
     */
    spi_bus_config_t bus_cfg = {
        .mosi_io_num = PIN_NUM_MOSI,
        .miso_io_num = PIN_NUM_MISO,
        .sclk_io_num = PIN_NUM_CLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1
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
        printf("Error montando sistema de archivos en SD\n");
        return -1;
    }

    printf("Tarjeta SD inicializada correctamente\n");

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