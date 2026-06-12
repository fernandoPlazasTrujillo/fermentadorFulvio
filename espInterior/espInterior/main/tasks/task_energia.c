/**
 * @file task_energia.c
 * @brief Implementación de la gestión energética del sistema.
 *
 * Esta tarea coordina el ciclo operativo general del sistema
 * de fermentación y gestiona la transición hacia modos de
 * bajo consumo.
 *
 * El flujo de operación sigue la secuencia:
 *
 * Medir
 *   ↓
 * Procesar
 *   ↓
 * Actuar
 *   ↓
 * Registrar
 *   ↓
 * Dormir
 *
 * La sincronización entre tareas se realiza mediante
 * notificaciones de FreeRTOS, permitiendo una ejecución
 * eficiente y evitando ciclos de espera activa.
 *
 * Funciones principales:
 * - Inicio de ciclos de adquisición.
 * - Coordinación entre tareas.
 * - Espera de finalización de procesos.
 * - Activación del modo Deep Sleep.
 *
 * Mecanismos FreeRTOS utilizados:
 * - Notificaciones entre tareas.
 *
 * Modos de bajo consumo:
 * - Deep Sleep.
 *
 * @author
 * Fernando Plazas Trujillo
 * Isabella Ordoñez
 * Juan Daniel Constain
 */

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"
#include "esp_sleep.h"

/**
 * @brief Handle de la tarea de sensores.
 *
 * Utilizado para iniciar un nuevo ciclo de adquisición.
 */
extern TaskHandle_t task_sensores_handle;

/**
 * @brief Etiqueta utilizada para mensajes de depuración.
 */
static const char *TAG = "ENERGIA";

/**
 * @brief Tarea principal de gestión energética.
 *
 * La tarea permanece bloqueada esperando una notificación
 * que indique el inicio de un nuevo ciclo operativo.
 *
 * Flujo de ejecución:
 * - Esperar activación.
 * - Iniciar adquisición de sensores.
 * - Esperar finalización de procesos.
 * - Entrar en modo Deep Sleep.
 *
 * La coordinación se realiza mediante notificaciones
 * directas de FreeRTOS para minimizar el uso de memoria
 * y CPU.
 *
 * @param pvParameters Parámetros de la tarea (no utilizados).
 */
void task_energia(void *pvParameters)
{
    while (1)
    {
        /**
         * Esperar inicio de ciclo.
         */
        ulTaskNotifyTake(
            pdTRUE,
            portMAX_DELAY);

        ESP_LOGI(TAG, "Ciclo iniciado");

        /**
         * Activar adquisición de sensores.
         */
        xTaskNotifyGive(
            task_sensores_handle);

        /**
         * Esperar finalización de procesamiento
         * y actuación.
         *
         * Actualmente se utiliza un tiempo máximo
         * de espera como mecanismo de sincronización.
         */
        ulTaskNotifyTake(
            pdTRUE,
            pdMS_TO_TICKS(25000));

        /**
         * Esperar finalización de tareas pendientes
         * como almacenamiento o comunicaciones.
         */
        ulTaskNotifyTake(
            pdTRUE,
            pdMS_TO_TICKS(5000));

        ESP_LOGI(TAG, "Entrando en deep sleep");

        /**
         * Transición al modo Deep Sleep.
         *
         * El sistema permanecerá suspendido hasta
         * que ocurra el evento de despertar configurado.
         */
        esp_deep_sleep_start();
    }
}