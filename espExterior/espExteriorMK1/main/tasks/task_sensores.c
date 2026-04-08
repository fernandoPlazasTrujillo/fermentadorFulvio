/**
 * @file task_sensores.c
 * @brief Tarea de adquisición de datos ambientales.
 *
 * Esta tarea realiza una medición única de los sensores del sistema:
 * - Sensor DHT11 (temperatura y humedad)
 * - RTC DS3231 (fecha y hora)
 *
 * Luego:
 * - Empaqueta los datos en una estructura
 * - Envía la información a otras tareas mediante colas
 * - Finaliza su ejecución
 *
 * Flujo de ejecución:
 * 1. Lectura del sensor DHT11
 * 2. Lectura del RTC
 * 3. Validación de datos
 * 4. Envío a colas:
 *    - cola_datos → task_logger
 *    - cola_display → task_display
 * 5. Eliminación de la tarea
 *
 * Comunicación:
 * - Salida: cola_datos, cola_display
 *
 * @note Esta tarea es de tipo "one-shot" (ejecución única).
 *
 * @warning No implementa control periódico ni sincronización con RTC
 * mediante interrupciones (esto puede mejorarse en futuras versiones).
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
#include "dht11.h"
#include "ds3231.h"

/**
 * @brief Tarea de lectura de sensores
 *
 * @param pvParameters Parámetros de la tarea (no utilizados)
 */
void task_sensores(void *pvParameters)
{
    /**
     * @brief Estructura principal de datos ambientales
     */
    datos_ambiente_t datos;

    /**
     * @brief Estructuras auxiliares de sensores
     */
    dht11_data_t dht;
    rtc_time_t rtc;

    printf("Ejecutando medicion de sensores...\n");

    // ===============================
    // LECTURA DEL SENSOR DHT11
    // ===============================

    /**
     * @brief Lectura de temperatura y humedad
     */
    if (dht11_read(&dht) == 0)
    {
        datos.temperatura = dht.temperatura;
        datos.humedad = dht.humedad;
    }
    else
    {
        /**
         * @brief Valores de error en caso de fallo
         */
        datos.temperatura = -1;
        datos.humedad = -1;
    }

    // ===============================
    // LECTURA DEL RTC DS3231
    // ===============================

    /**
     * @brief Lectura de fecha y hora
     */
    if (ds3231_read_time(&rtc) == 0)
    {
        datos.hora    = rtc.horas;
        datos.minuto  = rtc.minutos;
        datos.segundo = rtc.segundos;

        datos.dia  = rtc.dia;
        datos.mes  = rtc.mes;
        datos.anio = rtc.anio;
    }
    else
    {
        /**
         * @brief Valores por defecto en caso de error
         */
        datos.hora = 0;
        datos.minuto = 0;
        datos.segundo = 0;
        datos.dia = 0;
        datos.mes = 0;
        datos.anio = 0;
    }

    // ===============================
    // ENVÍO DE DATOS A OTRAS TAREAS
    // ===============================

    /**
     * @brief Envío a tarea de logging (SD)
     */
    xQueueSend(cola_datos, &datos, portMAX_DELAY);

    /**
     * @brief Envío a tarea de visualización (LCD)
     */
    xQueueSend(cola_display, &datos, portMAX_DELAY);

    printf("Medicion completada\n");

    // ===============================
    // FINALIZACIÓN DE LA TAREA
    // ===============================

    /**
     * @brief Eliminación de la tarea tras ejecución única
     */
    vTaskDelete(NULL);

    /**
     * @brief Bucle de seguridad (no debería ejecutarse)
     */
    while (1)
    {
        vTaskDelay(portMAX_DELAY);
    }
}