/**
 * @file task_display.c
 * @brief Implementación de la tarea de visualización del sistema.
 *
 * Esta tarea recibe información mediante una cola FreeRTOS y
 * actualiza la pantalla OLED con el estado actual del proceso
 * de fermentación.
 *
 * Información mostrada:
 * - Temperatura.
 * - pH.
 * - Concentración de CO2.
 * - Estado de WiFi.
 * - Estado de MQTT.
 * - Estado de la tarjeta SD.
 * - Hora del sistema.
 *
 * La tarea se encarga exclusivamente de la presentación de datos,
 * manteniendo desacoplada la interfaz de usuario de las tareas de
 * adquisición, control y comunicación.
 *
 * Arquitectura:
 *
 * queue_display
 *        |
 *        v
 *   task_display
 *        |
 *        v
 *      OLED
 *
 * Mecanismos FreeRTOS utilizados:
 * - Colas para recepción de datos.
 *
 * @author
 * Fernando Plazas Trujillo
 * Isabella Ordoñez
 * Juan Daniel Constain
 */

#include "tasks/task_display.h"

#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "drivers/oled_display.h"

#include "queues.h"
#include "utils/data_types.h"

/**
 * @brief Tarea encargada de actualizar la pantalla OLED.
 *
 * Espera datos provenientes de queue_display y muestra
 * información relevante del proceso de fermentación.
 *
 * La tarea permanece bloqueada hasta recibir nuevos datos,
 * minimizando el consumo de CPU.
 *
 * Información visualizada:
 * - Temperatura.
 * - pH.
 * - CO2.
 * - Estado de WiFi.
 * - Estado de MQTT.
 * - Estado de la tarjeta SD.
 * - Hora actual.
 *
 * @param pvParameters Parámetros de la tarea (no utilizados).
 */
void task_display(void *pvParameters)
{
    display_data_t data;

    char line[32];

    /**
     * Inicialización de la pantalla OLED.
     */
    oled_init();
    oled_clear();

    while (1)
    {
        if (xQueueReceive(
                queue_display,
                &data,
                portMAX_DELAY))
        {
            /**
             * Limpiar pantalla antes de redibujar.
             */
            oled_clear();

            // =====================================================
            // TEMPERATURA
            // =====================================================

            snprintf(
                line,
                sizeof(line),
                "T: %.1f C",
                data.temperatura);

            oled_draw_string(
                0,
                0,
                line);

            // =====================================================
            // PH
            // =====================================================

            snprintf(
                line,
                sizeof(line),
                "PH: %.2f",
                data.ph);

            oled_draw_string(
                0,
                12,
                line);

            // =====================================================
            // CO2
            // =====================================================

            snprintf(
                line,
                sizeof(line),
                "CO2: %.2f",
                data.co2);

            oled_draw_string(
                0,
                24,
                line);

            // =====================================================
            // ESTADOS DEL SISTEMA
            // =====================================================

            snprintf(
                line,
                sizeof(line),
                "W:%s M:%s S:%s",
                data.wifi_ok ? "OK" : "OFF",
                data.mqtt_ok ? "OK" : "OFF",
                data.sd_ok   ? "OK" : "OFF");

            oled_draw_string(
                0,
                36,
                line);

            // =====================================================
            // HORA
            // =====================================================

            printf(
                "DISPLAY -> %02d:%02d:%02d\n",
                data.datetime.hours,
                data.datetime.minutes,
                data.datetime.seconds);

            snprintf(
                line,
                sizeof(line),
                "%02d:%02d:%02d",
                data.datetime.hours,
                data.datetime.minutes,
                data.datetime.seconds);

            oled_draw_string(
                0,
                48,
                line);

            /**
             * Actualizar contenido físico de la pantalla.
             */
            oled_update();
        }
    }
}