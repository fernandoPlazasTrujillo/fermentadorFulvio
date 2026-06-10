/**
 * @file sd_card.h
 * @brief Módulo para manejo de tarjeta SD mediante SPI.
 * 
 * Permite inicializar la tarjeta y escribir datos en un archivo.
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
 * Configura el bus SPI y monta el sistema de archivos FAT.
 * 
 * @return ESP_OK si la tarjeta se monta correctamente.
 */
esp_err_t sd_card_init(void);

/**
 * @brief Escribe una línea en el archivo log.csv.
 * 
 * La escritura es protegida mediante mutex para evitar
 * conflictos en acceso SPI.
 * 
 * @param line Cadena a escribir en el archivo.
 * @return ESP_OK si la operación fue exitosa.
 */
esp_err_t sd_card_write_line(const char *line);

/**
 * @brief Guarda un payload MQTT no publicado para reintento posterior.
 */
esp_err_t sd_card_append_pending_mqtt(const char *payload);

/**
 * @brief Verifica si hay payloads MQTT pendientes en la SD.
 */
bool sd_card_pending_mqtt_exists(void);

/**
 * @brief Abre el archivo de payloads MQTT pendientes para lectura.
 */
FILE *sd_card_open_pending_mqtt_read(void);

/**
 * @brief Lee una linea del archivo de payloads MQTT pendientes.
 */
char *sd_card_read_pending_mqtt_line(FILE *file, char *buffer, size_t size);

/**
 * @brief Cierra el archivo de payloads MQTT pendientes.
 */
void sd_card_close_pending_mqtt(FILE *file);

/**
 * @brief Elimina el archivo de payloads MQTT pendientes.
 */
esp_err_t sd_card_delete_pending_mqtt(void);

bool sd_card_is_mounted(void);

#endif
