    #include <stdio.h>
#include <string.h>

#include "esp_log.h"
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"
#include "driver/spi_common.h"
#include "driver/sdspi_host.h"

#include "sd_card.h"

static const char *TAG = "SD";

#define PIN_MISO 19
#define PIN_MOSI 23
#define PIN_CLK  18
#define PIN_CS   5

static const char mount_point[] = "/sdcard";

static FILE *log_file = NULL;

esp_err_t sd_card_init(void)
{
    esp_err_t ret;

    sdmmc_host_t host = SDSPI_HOST_DEFAULT();

    spi_bus_config_t bus_cfg = {
        .mosi_io_num = PIN_MOSI,
        .miso_io_num = PIN_MISO,
        .sclk_io_num = PIN_CLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
    };

    ret = spi_bus_initialize(host.slot, &bus_cfg, SDSPI_DEFAULT_DMA);
    if (ret != ESP_OK) return ret;

    sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
    slot_config.gpio_cs = PIN_CS;
    slot_config.host_id = host.slot;

    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = false,
        .max_files = 5,
    };

    sdmmc_card_t *card;

    ret = esp_vfs_fat_sdspi_mount(mount_point, &host, &slot_config, &mount_config, &card);

    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Error montando SD");
        return ret;
    }

    ESP_LOGI(TAG, "SD montada");

    // abrir archivo UNA VEZ
    log_file = fopen("/sdcard/log.csv", "a");

    if (log_file == NULL) {
        ESP_LOGE(TAG, "Error abriendo archivo");
        return ESP_FAIL;
    }

    return ESP_OK;
}

esp_err_t sd_card_write_line(const char *line)
{
    if (log_file == NULL) {
        return ESP_FAIL;
    }

    fprintf(log_file, "%s\n", line);

    // flush para asegurar escritura
    fflush(log_file);

    return ESP_OK;
}