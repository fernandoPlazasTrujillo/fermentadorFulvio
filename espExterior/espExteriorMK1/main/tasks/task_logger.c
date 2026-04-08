/**
 * @file task_logger.c
 * @brief Tarea encargada del almacenamiento de datos en tarjeta SD.
 *
 * Esta tarea recibe datos ambientales desde una cola y los almacena
 * en una tarjeta SD en formato CSV.
 *
 * Responsabilidades:
 * - Recibir datos desde otras tareas (sensores/control)
 * - Formatear la información en una línea estructurada
 * - Guardar los datos en la SD
 * - Mostrar información de depuración en consola
 *
 * Flujo de ejecución:
 * 1. Espera datos en la cola (bloqueante)
 * 2. Formatea los datos en formato CSV
 * 3. Imprime en consola (debug)
 * 4. Guarda en tarjeta SD
 *
 * Comunicación:
 * - Entrada: cola_datos (Queue)
 *
 * @note Esta tarea es bloqueante sobre la cola, lo que optimiza
 * el uso de CPU y energía.
 *
 * @warning El acceso a la SD no es thread-safe. Se recomienda usar
 * un mutex SPI si múltiples tareas acceden al bus.
 *
 * @authors
 * - Fernando Plazas
 * - Isabella Ordoñez
 * - Juan Daniel Constain
 */

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdio.h>

#include "queues.h"
#include "common.h"
#include "sd_card.h"

/**
 * @brief Tarea de registro de datos en SD
 *
 * @param pvParameters Parámetros de la tarea (no utilizados)
 */
void task_logger(void *pvParameters)
{
    /**
     * @brief Estructura para recibir datos desde la cola
     */
    datos_ambiente_t datos;

    /**
     * @brief Buffer para línea CSV
     */
    char linea[100];

    while (1)
    {
        /**
         * @brief Espera datos desde la cola
         *
         * La tarea permanece bloqueada hasta recibir información,
         * evitando uso innecesario del procesador.
         */
        if (xQueueReceive(cola_datos, &datos, portMAX_DELAY))
        {
            // ===============================
            // FORMATO DE DATOS (CSV)
            // ===============================

            /**
             * @brief Generación de línea en formato CSV
             *
             * Formato:
             * HH:MM:SS,DD/MM/YYYY,temperatura,humedad
             */
            snprintf(linea, sizeof(linea),
                     "%02d:%02d:%02d,%02d/%02d/%04d,%.2f,%.2f",
                     datos.hora,
                     datos.minuto,
                     datos.segundo,
                     datos.dia,
                     datos.mes,
                     datos.anio,
                     datos.temperatura,
                     datos.humedad);

            // ===============================
            // SALIDA POR CONSOLA (DEBUG)
            // ===============================

            /**
             * @brief Impresión en consola para monitoreo
             */
            printf("LOG -> %s\n", linea);

            // ===============================
            // ESCRITURA EN TARJETA SD
            // ===============================

            /**
             * @brief Guardado de datos en la SD
             */
            if (sd_write_line(linea) != 0)
            {
                printf("Error escribiendo en SD\n");
            }
            else
            {
                printf("Dato guardado en SD\n");
            }
        }
    }
}