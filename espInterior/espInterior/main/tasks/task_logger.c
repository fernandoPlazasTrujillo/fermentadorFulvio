/**
 * @file task_logger.c
 * @brief Implementación de la tarea de registro de datos.
 *
 * Esta tarea recibe mediciones provenientes de los sensores,
 * genera registros en formato CSV y los almacena de forma
 * persistente en la tarjeta SD.
 *
 * Funciones principales:
 * - Recepción de datos mediante colas FreeRTOS.
 * - Formateo de registros CSV.
 * - Escritura en memoria SD.
 * - Generación de historial de fermentación.
 *
 * Arquitectura:
 *
 * queue_logger
 *        |
 *        v
 *   task_logger
 *        |
 *        v
 *     SD Card
 *
 * La tarea se ejecuta de forma independiente para evitar que
 * las operaciones de almacenamiento bloqueen la adquisición
 * de sensores o la lógica de control.
 *
 * Mecanismos FreeRTOS utilizados:
 * - Colas para comunicación entre tareas.
 *
 * @author
 * Fernando Plazas Trujillo
 * Isabella Ordoñez
 * Juan Daniel Constain
 */

#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "esp_log.h"

#include "utils/data_types.h"
#include "drivers/sd_card.h"
#include "queues.h"

/**
 * @brief Etiqueta utilizada para mensajes de depuración.
 */
static const char *TAG = "LOGGER";

/**
 * @brief Tarea principal de registro de datos.
 *
 * Espera mediciones provenientes de queue_logger,
 * genera una línea en formato CSV y la almacena
 * en la tarjeta SD.
 *
 * Cada registro contiene:
 * - Fecha.
 * - Hora.
 * - Temperatura.
 * - pH.
 * - CO2 procesado.
 * - CO2 crudo.
 * - Voltaje.
 * - Corriente.
 *
 * La tarea permanece bloqueada mientras no existan
 * nuevos datos disponibles, optimizando el uso de CPU.
 *
 * @param pvParameters Parámetros de la tarea (no utilizados).
 */
void task_logger(void *pvParameters)
{
    sensor_data_t data;

    /**
     * Buffer utilizado para generar la línea CSV.
     */
    char line[160];

    /**
     * Inicializar tarjeta SD.
     */
    if (sd_card_init() != ESP_OK)
    {
        ESP_LOGE(TAG, "SD no inicializada");
    }

    while (1)
    {
        /**
         * Esperar nuevos datos para registrar.
         */
        if (xQueueReceive(
                queue_logger,
                &data,
                portMAX_DELAY))
        {
            /**
             * Generar registro CSV.
             */
            snprintf(
                line,
                sizeof(line),
                "%02d-%02d-%04d, %02d:%02d:%02d, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f",
                data.datetime.day,
                data.datetime.month,
                data.datetime.year,
                data.datetime.hours,
                data.datetime.minutes,
                data.datetime.seconds,
                data.temperatura,
                data.ph,
                data.co2,
                data.co2_raw,
                data.voltaje,
                data.corriente);

            ESP_LOGI(TAG, "LOG -> %s", line);

            /**
             * Almacenar registro en tarjeta SD.
             */
            if (sd_card_write_line(line) != ESP_OK)
            {
                ESP_LOGE(TAG, "Error escribiendo en SD");
            }
            else
            {
                ESP_LOGI(TAG, "Dato guardado en SD");
            }
        }
    }
}