/**
 * @file sd_card.h
 * @brief Módulo de manejo de tarjeta SD mediante interfaz SPI.
 *
 * Este módulo proporciona funciones para:
 * - Inicializar la tarjeta SD
 * - Montar el sistema de archivos FAT
 * - Escribir datos en un archivo de log
 *
 * El sistema utiliza la capa VFS de ESP-IDF para acceder a la SD
 * como un sistema de archivos estándar.
 *
 * @note Este módulo no es thread-safe. Si múltiples tareas acceden
 * a la SD, se debe usar un mutex para proteger el bus SPI.
 *
 * @warning El acceso concurrente sin sincronización puede causar
 * corrupción de datos.
 *
 * @authors
 * - Fernando Plazas
 * - Isabella Ordoñez
 * - Juan Daniel Constain
 */

#pragma once

#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>

/**
 * @brief Inicializa la tarjeta SD
 *
 * Configura el bus SPI, inicializa el dispositivo y monta
 * el sistema de archivos FAT en la ruta "/sdcard".
 *
 * @return int Código de estado:
 * - 0: inicialización exitosa
 * - -1: error en inicialización o montaje
 */
int sd_init(void);

/**
 * @brief Escribe una línea en el archivo de log
 *
 * Abre (o crea) el archivo "log.txt" en la SD y añade
 * una nueva línea de texto.
 *
 * @param line Cadena a escribir en el archivo
 *
 * @return int Código de estado:
 * - 0: escritura exitosa
 * - -1: error al abrir el archivo
 *
 * @note El archivo se abre en modo append ("a")
 */
int sd_write_line(const char *line);

int sd_append_pending_mqtt(const char *payload);

/**
 * @brief Verifica si existe el archivo de mensajes MQTT pendientes.
 *
 * @return true si existe "/sdcard/test.txt", false en caso contrario.
 */
bool sd_pending_mqtt_exists(void);

/**
 * @brief Abre el archivo de mensajes MQTT pendientes en modo lectura.
 *
 * @return FILE* Puntero al archivo abierto, o NULL si no se pudo abrir.
 */
FILE *sd_open_pending_mqtt_read(void);

/**
 * @brief Lee una linea del archivo de mensajes MQTT pendientes.
 *
 * @param file Archivo abierto previamente con sd_open_pending_mqtt_read().
 * @param buffer Buffer donde se almacenara la linea leida.
 * @param size Tamano del buffer.
 *
 * @return char* El mismo buffer si se leyo una linea, o NULL al llegar al fin
 * del archivo o si ocurre un error.
 */
char *sd_read_pending_mqtt_line(FILE *file, char *buffer, size_t size);

/**
 * @brief Cierra el archivo de mensajes MQTT pendientes.
 *
 * @param file Archivo abierto a cerrar.
 */
void sd_close_pending_mqtt(FILE *file);

/**
 * @brief Elimina el archivo de mensajes MQTT pendientes.
 *
 * @return int Codigo de estado:
 * - 0: eliminacion exitosa o archivo inexistente
 * - -1: error eliminando el archivo
 */
int sd_delete_pending_mqtt(void);
