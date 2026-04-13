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

#endif