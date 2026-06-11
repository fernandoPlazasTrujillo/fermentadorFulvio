/**
 * @file sd_card.c
 * @brief Implementación del módulo de almacenamiento en tarjeta SD.
 *
 * Este módulo implementa la gestión de una tarjeta SD conectada
 * mediante interfaz SPI utilizando el sistema de archivos FAT
 * proporcionado por ESP-IDF.
 *
 * Funcionalidades implementadas:
 * - Inicialización del bus SPI.
 * - Montaje del sistema de archivos FAT.
 * - Registro persistente de datos.
 * - Almacenamiento de mensajes MQTT pendientes.
 * - Recuperación de mensajes pendientes tras reconexión.
 *
 * @section sd_role Rol dentro del sistema
 *
 * Este módulo forma parte del subsistema de persistencia del
 * sistema de fermentación de café.
 *
 * Se utiliza para:
 * - Almacenar mediciones ambientales.
 * - Registrar eventos del sistema.
 * - Mantener mensajes MQTT durante pérdidas de conectividad.
 *
 * @section mqtt_store_forward Estrategia Store-and-Forward
 *
 * Cuando el broker MQTT no está disponible:
 *
 * Sensor -> SD
 *
 * Al recuperarse la conexión:
 *
 * SD -> MQTT Broker
 *
 * Esto garantiza que no se pierdan mediciones durante fallos
 * temporales de red.
 *
 * @note El acceso concurrente debe protegerse mediante mutex.
 *
 * @warning Escrituras simultáneas sin sincronización pueden
 * provocar corrupción del sistema de archivos FAT.
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

/**
 * @brief GPIO utilizado como línea MISO del bus SPI.
 */
#define PIN_NUM_MISO 19 /**< Pin MISO */

/**
 * @brief GPIO utilizado como línea MOSI del bus SPI.
 */
#define PIN_NUM_MOSI 23 /**< Pin MOSI */

/**
 * @brief GPIO utilizado como reloj SPI.
 */
#define PIN_NUM_CLK  18 /**< Pin CLK */

/**
 * @brief GPIO utilizado como Chip Select de la SD.
 */
#define PIN_NUM_CS   4 /**< Pin Chip Select */

/**
 * @brief Frecuencia máxima del bus SPI en kHz.
 */
#define SD_SPI_MAX_FREQ_KHZ 4000

/**
 * @brief Ruta del archivo utilizado para almacenar mensajes MQTT pendientes.
 */
#define SD_PENDING_MQTT_PATH "/sdcard/test.txt"

/**
 * @brief Inicializa la tarjeta SD y monta el sistema de archivos FAT.
 *
 * Configura el bus SPI, registra el dispositivo SD y monta
 * el sistema de archivos en la ruta "/sdcard".
 *
 * Esta función debe ejecutarse antes de cualquier operación
 * de lectura o escritura.
 *
 * @retval 0 Inicialización exitosa.
 * @retval -1 Error de inicialización o montaje.
 *
 * @note Debe ejecutarse una única vez durante el arranque.
 */
int sd_init(void)
{
    esp_err_t ret;

    sdmmc_host_t host = SDSPI_HOST_DEFAULT();
    host.max_freq_khz = SD_SPI_MAX_FREQ_KHZ;

    spi_bus_config_t bus_cfg = {
        .mosi_io_num = PIN_NUM_MOSI,
        .miso_io_num = PIN_NUM_MISO,
        .sclk_io_num = PIN_NUM_CLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 4000
    };

    ret = spi_bus_initialize(host.slot, &bus_cfg, SDSPI_DEFAULT_DMA);
    if (ret != ESP_OK)
    {
        printf("Error inicializando bus SPI\n");
        return -1;
    }

    sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
    slot_config.gpio_cs = PIN_NUM_CS;
    slot_config.host_id = host.slot;

    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = false,
        .max_files = 5
    };

    sdmmc_card_t *card;

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

/**
 * @brief Escribe una línea en el archivo principal de registro.
 *
 * Abre el archivo de log en modo append, añade una nueva línea
 * y cierra el archivo para garantizar la integridad de los datos.
 *
 * @param[in] line Cadena de texto a almacenar.
 *
 * @retval 0 Escritura exitosa.
 * @retval -1 Error al abrir el archivo.
 */
int sd_write_line(const char *line)
{

    FILE *f = fopen("/sdcard/log.txt", "a");

    if (f == NULL)
    {
        printf("Error abriendo archivo en SD\n");
        return -1;
    }

    fprintf(f, "%s\n", line);

    fclose(f);

    return 0;
}

/**
 * @brief Almacena un mensaje MQTT pendiente de transmisión.
 *
 * Cuando el broker MQTT no está disponible, el payload se guarda
 * en un archivo temporal dentro de la tarjeta SD.
 *
 * Posteriormente podrá ser reenviado cuando la conectividad
 * sea restaurada.
 *
 * @param[in] payload Mensaje MQTT serializado.
 *
 * @retval 0 Escritura exitosa.
 * @retval -1 Error de almacenamiento.
 */
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

/**
 * @brief Verifica la existencia del archivo de mensajes pendientes.
 *
 * Determina si existen datos almacenados para retransmisión MQTT.
 *
 * @retval true El archivo existe.
 * @retval false El archivo no existe.
 */
bool sd_pending_mqtt_exists(void)
{
    struct stat file_stat;

    return stat(SD_PENDING_MQTT_PATH, &file_stat) == 0;
}

/**
 * @brief Abre el archivo de mensajes MQTT pendientes.
 *
 * Permite iniciar el proceso de recuperación y retransmisión
 * de mensajes almacenados.
 *
 * @return FILE* Descriptor del archivo.
 * @return NULL Error al abrir el archivo.
 */
FILE *sd_open_pending_mqtt_read(void)
{
    FILE *f = fopen(SD_PENDING_MQTT_PATH, "r");

    if (f == NULL)
    {
        printf("ERROR: No se pudo abrir %s en lectura. errno=%d\n", SD_PENDING_MQTT_PATH, errno);
    }

    return f;
}

/**
 * @brief Lee una línea del archivo de mensajes pendientes.
 *
 * Recupera un payload MQTT previamente almacenado en la SD.
 *
 * @param[in] file Archivo abierto para lectura.
 * @param[out] buffer Buffer de destino.
 * @param[in] size Tamaño del buffer.
 *
 * @return char* Puntero al buffer si la lectura fue exitosa.
 * @return NULL Fin de archivo o error.
 */
char *sd_read_pending_mqtt_line(FILE *file, char *buffer, size_t size)
{
    if (file == NULL || buffer == NULL || size == 0)
    {
        return NULL;
    }

    return fgets(buffer, size, file);
}
/**
 * @brief Cierra el archivo de mensajes MQTT pendientes.
 *
 * Libera los recursos asociados al descriptor de archivo.
 *
 * @param[in] file Archivo previamente abierto.
 */
void sd_close_pending_mqtt(FILE *file)
{
    if (file != NULL)
    {
        fclose(file);
    }
}

/**
 * @brief Elimina el archivo de mensajes MQTT pendientes.
 *
 * Debe ejecutarse después de retransmitir exitosamente todos
 * los mensajes almacenados.
 *
 * @retval 0 Eliminación exitosa.
 * @retval -1 Error eliminando el archivo.
 */
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
