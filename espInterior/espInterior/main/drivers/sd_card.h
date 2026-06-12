/**
 * @file sd_card.h
 * @brief Módulo para gestión de tarjeta SD mediante interfaz SPI.
 *
 * Este módulo proporciona funciones para inicializar la tarjeta SD,
 * almacenar registros históricos del sistema y gestionar mensajes MQTT
 * pendientes cuando no existe conectividad de red.
 *
 * Funcionalidades:
 * - Montaje del sistema de archivos FAT.
 * - Escritura de registros CSV.
 * - Almacenamiento temporal de mensajes MQTT.
 * - Recuperación de mensajes pendientes.
 * - Verificación del estado de montaje.
 *
 * El acceso a la tarjeta SD es protegido mediante mecanismos
 * de sincronización para evitar conflictos de acceso concurrente
 * al bus SPI.
 *
 * @author
 * Fernando Plazas Trujillo
 * Isabella Ordoñez
 * Juan Daniel Constain
 */

#ifndef SD_CARD_H
#define SD_CARD_H

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

#include "esp_err.h"

/**
 * @brief Inicializa la tarjeta SD.
 *
 * Configura el bus SPI, monta el sistema de archivos FAT
 * y verifica la disponibilidad del dispositivo de almacenamiento.
 *
 * Esta función debe ejecutarse durante la inicialización
 * del sistema antes de realizar operaciones de lectura
 * o escritura.
 *
 * @return
 * - ESP_OK: Tarjeta montada correctamente.
 * - Código de error en caso de fallo.
 */
esp_err_t sd_card_init(void);

/**
 * @brief Escribe una línea en el archivo de registro.
 *
 * Añade una nueva línea al archivo CSV utilizado para
 * almacenar las variables del proceso de fermentación.
 *
 * El acceso al archivo se encuentra protegido para evitar
 * conflictos entre tareas concurrentes.
 *
 * @param line Cadena de texto a almacenar.
 *
 * @return
 * - ESP_OK: Escritura exitosa.
 * - Código de error en caso contrario.
 */
esp_err_t sd_card_write_line(const char *line);

/**
 * @brief Guarda un payload MQTT pendiente de publicación.
 *
 * Cuando no existe conectividad con el broker MQTT,
 * el mensaje se almacena temporalmente en la tarjeta SD
 * para su posterior retransmisión.
 *
 * @param payload Mensaje MQTT a almacenar.
 *
 * @return
 * - ESP_OK: Operación exitosa.
 * - Código de error en caso contrario.
 */
esp_err_t sd_card_append_pending_mqtt(const char *payload);

/**
 * @brief Verifica la existencia de mensajes MQTT pendientes.
 *
 * Comprueba si existe el archivo de almacenamiento temporal
 * utilizado para guardar mensajes no publicados.
 *
 * @return
 * - true: Existen mensajes pendientes.
 * - false: No existen mensajes pendientes.
 */
bool sd_card_pending_mqtt_exists(void);

/**
 * @brief Abre el archivo de mensajes MQTT pendientes.
 *
 * El archivo es abierto en modo lectura para permitir
 * la recuperación de mensajes almacenados previamente.
 *
 * @return Puntero al archivo abierto o NULL si ocurre un error.
 */
FILE *sd_card_open_pending_mqtt_read(void);

/**
 * @brief Lee una línea del archivo de mensajes MQTT pendientes.
 *
 * Obtiene el siguiente registro almacenado dentro del archivo.
 *
 * @param file Puntero al archivo abierto.
 * @param buffer Buffer donde se almacenará la línea leída.
 * @param size Tamaño del buffer.
 *
 * @return
 * - Puntero al buffer si la lectura fue exitosa.
 * - NULL si se alcanza el final del archivo o ocurre un error.
 */
char *sd_card_read_pending_mqtt_line(
    FILE *file,
    char *buffer,
    size_t size);

/**
 * @brief Cierra el archivo de mensajes MQTT pendientes.
 *
 * Libera los recursos asociados al archivo abierto.
 *
 * @param file Puntero al archivo.
 */
void sd_card_close_pending_mqtt(FILE *file);

/**
 * @brief Elimina el archivo de mensajes MQTT pendientes.
 *
 * Se utiliza una vez que todos los mensajes almacenados
 * han sido retransmitidos correctamente al broker MQTT.
 *
 * @return
 * - ESP_OK: Archivo eliminado correctamente.
 * - Código de error en caso contrario.
 */
esp_err_t sd_card_delete_pending_mqtt(void);

/**
 * @brief Verifica si la tarjeta SD se encuentra montada.
 *
 * Permite a otras tareas validar la disponibilidad del
 * sistema de almacenamiento antes de realizar operaciones
 * de lectura o escritura.
 *
 * @return
 * - true: Tarjeta montada correctamente.
 * - false: Tarjeta no disponible.
 */
bool sd_card_is_mounted(void);

#endif