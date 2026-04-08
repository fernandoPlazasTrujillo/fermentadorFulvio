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