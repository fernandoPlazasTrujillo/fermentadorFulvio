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
 *    - cola_datos   → task_logger
 *    - cola_display → task_display
 *    - cola_mqtt    → task_mqtt
 * 5. Eliminación de la tarea
 *
 * Comunicación:
 * - Salida:
 *   - cola_datos
 *   - cola_display
 *   - cola_mqtt
 * 
 * Aspectos de FreeRTOS utilizados:
 * - Queue para distribución de datos entre tareas.
 * - Ejecución one-shot mediante creación y destrucción dinámica.
 * - Bloqueo seguro mediante portMAX_DELAY durante el envío.
 *
 * @note Esta tarea es de tipo "one-shot" (ejecución única).
 *
 * @note La periodicidad de ejecución es gestionada por la tarea
 * principal del sistema, manteniendo esta tarea enfocada
 * exclusivamente en la adquisición de datos.
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

    datos_ambiente_t datos;


    dht11_data_t dht;
    rtc_time_t rtc;

    printf("Ejecutando medicion de sensores...\n");

    // ===============================
    // LECTURA DEL SENSOR DHT11
    // ===============================


    if (dht11_read(&dht) == 0)
    {
        datos.temperatura = dht.temperatura;
        datos.humedad = dht.humedad;
    }
    else
    {

        datos.temperatura = -1;
        datos.humedad = -1;
    }

    // ===============================
    // LECTURA DEL RTC DS3231
    // ===============================


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


    xQueueSend(cola_datos, &datos, portMAX_DELAY);


    xQueueSend(cola_display, &datos, portMAX_DELAY);

    xQueueSend(cola_mqtt, &datos, portMAX_DELAY);

    printf("Medicion completada\n");

    // ===============================
    // FINALIZACIÓN DE LA TAREA
    // ===============================


    vTaskDelete(NULL);


    while (1)
    {
        vTaskDelay(portMAX_DELAY);
    }
}
