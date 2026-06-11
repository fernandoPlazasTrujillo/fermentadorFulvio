/**
 * @file sd_card.h
 * @brief Módulo de gestión de tarjeta SD mediante interfaz SPI.
 *
 * Este módulo proporciona una capa de abstracción para el acceso
 * a una tarjeta SD utilizando el sistema de archivos FAT y la
 * capa VFS de ESP-IDF.
 *
 * Funcionalidades principales:
 * - Inicialización y montaje de la tarjeta SD.
 * - Registro persistente de datos de sensores.
 * - Almacenamiento de eventos del sistema.
 * - Buffer persistente para mensajes MQTT pendientes.
 *
 * @section sd_architecture Integración en el sistema
 *
 * Dentro del sistema de fermentación de café, este módulo cumple
 * dos funciones fundamentales:
 *
 * - Almacenamiento histórico de mediciones ambientales.
 * - Persistencia temporal de mensajes MQTT cuando la conexión
 *   con el broker no está disponible.
 *
 * Esta estrategia permite mantener la integridad de los datos
 * incluso ante fallos de red o reinicios inesperados.
 *
 * @note Este módulo no es thread-safe.
 * @note El acceso concurrente debe protegerse mediante mutex SPI.
 *
 * @warning El acceso simultáneo sin sincronización puede provocar
 * corrupción del sistema de archivos FAT.
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
 * @brief Inicializa y monta la tarjeta SD.
 *
 * Configura el bus SPI, registra el dispositivo SD y monta
 * el sistema de archivos FAT en la ruta "/sdcard".
 *
 * Esta función debe ejecutarse antes de cualquier operación
 * de lectura o escritura.
 *
 * @retval 0 Inicialización exitosa.
 * @retval -1 Error de inicialización o montaje.
 *
 * @note Debe ejecutarse una única vez durante el arranque
 * del sistema.
 */
int sd_init(void);

/**
 * @brief Añade una línea al archivo principal de registro.
 *
 * Abre (o crea) el archivo de log y agrega una nueva línea
 * de texto al final del archivo.
 *
 * Esta función es utilizada para almacenar mediciones,
 * eventos y registros del sistema.
 *
 * @param[in] line Cadena de texto a escribir.
 *
 * @retval 0 Escritura exitosa.
 * @retval -1 Error de acceso al archivo.
 */
int sd_write_line(const char *line);

/**
 * @brief Almacena un mensaje MQTT pendiente de transmisión.
 *
 * Cuando el sistema detecta que el broker MQTT no está disponible,
 * los mensajes se almacenan temporalmente en un archivo de respaldo
 * dentro de la tarjeta SD.
 *
 * Una vez restablecida la conexión, dichos mensajes pueden ser
 * reenviados al broker.
 *
 * @param[in] payload Mensaje MQTT serializado.
 *
 * @retval 0 Escritura exitosa.
 * @retval -1 Error al almacenar el mensaje.
 */
int sd_append_pending_mqtt(const char *payload);

/**
 * @brief Verifica la existencia del archivo de respaldo MQTT.
 *
 * Determina si existen mensajes pendientes almacenados
 * para ser retransmitidos al broker.
 *
 * @retval true El archivo existe.
 * @retval false El archivo no existe.
 */
bool sd_pending_mqtt_exists(void);

/**
 * @brief Abre el archivo de mensajes MQTT pendientes.
 *
 * Permite iniciar el proceso de lectura secuencial de los
 * mensajes almacenados durante una pérdida de conectividad.
 *
 * @return FILE* Descriptor del archivo abierto.
 * @return NULL Error al abrir el archivo.
 */
FILE *sd_open_pending_mqtt_read(void);

/**
 * @brief Lee un mensaje MQTT almacenado en la SD.
 *
 * Recupera una línea del archivo de respaldo y la almacena
 * en el buffer proporcionado.
 *
 * @param[in] file Archivo abierto para lectura.
 * @param[out] buffer Buffer de destino.
 * @param[in] size Tamaño del buffer.
 *
 * @return char* Puntero al buffer si la lectura fue exitosa.
 * @return NULL Fin de archivo o error.
 */
char *sd_read_pending_mqtt_line(FILE *file, char *buffer, size_t size);

/**
 * @brief Cierra el archivo de mensajes MQTT pendientes.
 *
 * Libera los recursos asociados al descriptor de archivo.
 *
 * @param[in] file Archivo previamente abierto.
 */
void sd_close_pending_mqtt(FILE *file);

/**
 * @brief Elimina el archivo de mensajes MQTT pendientes.
 *
 * Esta función debe ejecutarse una vez que todos los mensajes
 * almacenados hayan sido retransmitidos exitosamente al broker.
 *
 * @retval 0 Eliminación exitosa o archivo inexistente.
 * @retval -1 Error durante la eliminación.
 */
int sd_delete_pending_mqtt(void);
