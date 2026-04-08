/**
 * @file task_display.c
 * @brief Tarea encargada de la visualización de datos en LCD.
 *
 * Esta tarea recibe datos del sistema a través de una cola y los
 * muestra en una pantalla LCD 16x2.
 *
 * Responsabilidades:
 * - Formatear datos para visualización
 * - Actualizar la pantalla LCD
 * - Sincronizar con la tarea principal mediante notificaciones
 *
 * Flujo de ejecución:
 * 1. Espera datos desde la cola (bloqueante)
 * 2. Formatea la información (hora, fecha, temperatura, humedad)
 * 3. Actualiza el display
 * 4. Notifica a la tarea principal
 *
 * Comunicación:
 * - Entrada: cola_display (Queue)
 * - Salida: notificación a main_task_handle
 *
 * @note Esta tarea es bloqueante sobre la cola (portMAX_DELAY),
 * lo cual optimiza el consumo energético.
 *
 * @warning El acceso al LCD no es thread-safe. Esta tarea debe ser
 * la única que interactúe con el display o debe usarse un mutex.
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
#include "lcd_parallel.h"

/**
 * @brief Handle de la tarea principal
 *
 * Permite notificar al sistema que el ciclo de visualización ha finalizado.
 */
extern TaskHandle_t main_task_handle;

/**
 * @brief Tarea de visualización en LCD
 *
 * @param pvParameters Parámetros de la tarea (no utilizados)
 */
void task_display(void *pvParameters)
{
    /**
     * @brief Estructura para recibir datos desde la cola
     */
    datos_ambiente_t datos;

    /**
     * @brief Buffers para formateo de texto
     */
    char linea1[20];
    char linea2[20];
    char buffer1[20];
    char buffer2[20];

    /**
     * @brief Inicialización del LCD
     */
    lcd_init();

    /**
     * @brief Retardo inicial para estabilización
     */
    vTaskDelay(pdMS_TO_TICKS(50));

    while (1)
    {
        /**
         * @brief Espera datos desde la cola
         *
         * Bloquea indefinidamente hasta recibir información,
         * evitando consumo innecesario de CPU.
         */
        if (xQueueReceive(cola_display, &datos, portMAX_DELAY))
        {
            // ==========================
            // FORMATEO DE DATOS
            // ==========================

            /**
             * @brief Formato de fecha y hora
             */
            snprintf(linea1, sizeof(linea1),
                     "%02d:%02d:%02d %02d/%02d",
                     datos.hora,
                     datos.minuto,
                     datos.segundo,
                     datos.dia,
                     datos.mes);

            /**
             * @brief Formato de variables ambientales
             */
            snprintf(linea2, sizeof(linea2),
                     "T:%.1f H:%.1f",
                     datos.temperatura,
                     datos.humedad);

            /**
             * @brief Ajuste a 16 caracteres (ancho del LCD)
             */
            snprintf(buffer1, sizeof(buffer1), "%-16s", linea1);
            snprintf(buffer2, sizeof(buffer2), "%-16s", linea2);

            // ==========================
            // ACTUALIZACIÓN DEL DISPLAY
            // ==========================

            /**
             * @brief Mostrar primera línea
             */
            lcd_set_cursor(0, 0);
            lcd_print(buffer1);

            /**
             * @brief Mostrar segunda línea
             */
            lcd_set_cursor(0, 1);
            lcd_print(buffer2);

            // ==========================
            // SINCRONIZACIÓN
            // ==========================

            /**
             * @brief Notifica a la tarea principal
             *
             * Indica que la fase de visualización ha terminado,
             * permitiendo avanzar en el ciclo del sistema.
             */
            xTaskNotifyGive(main_task_handle);
        }
    }
}