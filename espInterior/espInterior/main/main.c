/**
 * @file main.c
 * @brief Punto de entrada del sistema de fermentación basado en ESP32.
 *
 * Este módulo realiza la inicialización completa del sistema,
 * configura los periféricos necesarios, crea las tareas FreeRTOS
 * y pone en marcha el ciclo operativo de adquisición, control,
 * almacenamiento y comunicación.
 *
 * Funciones principales:
 * - Inicialización de hardware.
 * - Inicialización de red WiFi.
 * - Configuración del RTC DS3231.
 * - Configuración de wakeup para Deep Sleep.
 * - Creación de colas FreeRTOS.
 * - Creación de tareas del sistema.
 * - Inicio del ciclo operativo.
 *
 * Arquitectura general:
 *
 *              task_energia
 *                    |
 *                    v
 *             task_sensores
 *                    |
 *        +-----------+-----------+
 *        |           |           |
 *        v           v           v
 *   task_control task_logger task_mqtt
 *        |                       |
 *        v                       v
 * task_actuadores           Broker MQTT
 *        |
 *        v
 *   Actuadores
 *
 *                 |
 *                 v
 *          task_display
 *
 * Mecanismos FreeRTOS utilizados:
 * - Tareas.
 * - Colas.
 * - Notificaciones.
 *
 * Gestión energética:
 * - Deep Sleep.
 * - Wakeup mediante RTC DS3231.
 *
 * @author
 * Fernando Plazas Trujillo
 * Isabella Ordoñez
 * Juan Daniel Constain
 */

#include <stdio.h>
#include <stdbool.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"
#include "esp_sleep.h"

// HAL
#include "hal/i2c_manager.h"
#include "hal/adc_manager.h"

// Drivers
#include "drivers/rtc_ds3231.h"

// Tasks
#include "tasks/task_sensores.h"
#include "tasks/task_control.h"
#include "tasks/task_actuadores.h"
#include "tasks/task_logger.h"
#include "tasks/task_energia.h"
#include "tasks/task_mqtt.h"
#include "tasks/task_display.h"

// Network
#include "wifi/wifi_manager.h"

// Queues
#include "queues.h"

// Drivers
#include "drivers/rtc_ds3231.h"
#include "drivers/oled_display.h"

// =====================================================
// HANDLES DE TAREAS
// =====================================================

/**
 * @brief Handle de la tarea de sensores.
 *
 * Utilizado por task_energia para iniciar
 * nuevos ciclos de adquisición.
 */
TaskHandle_t task_sensores_handle = NULL;

/**
 * @brief Handle de la tarea de energía.
 *
 * Utilizado para coordinar el ciclo general
 * del sistema.
 */
TaskHandle_t task_energia_handle = NULL;

/**
 * @brief Etiqueta utilizada para mensajes de depuración.
 */
static const char *TAG = "MAIN";

/**
 * @brief Tiempo máximo de espera para conexión WiFi.
 */
#define WIFI_CONNECT_TIMEOUT_MS 10000

/**
 * @brief Función principal del sistema.
 *
 * Realiza toda la secuencia de inicialización:
 *
 * 1. Conexión WiFi.
 * 2. Inicialización de hardware.
 * 3. Configuración del RTC.
 * 4. Configuración de Deep Sleep.
 * 5. Creación de colas.
 * 6. Creación de tareas.
 * 7. Inicio del ciclo operativo.
 */
void app_main(void)
{
    ESP_LOGI(TAG, "Iniciando sistema...");

    // =====================================================
    // INICIALIZACIÓN WIFI
    // =====================================================

    wifi_init();

    bool wifi_connected =
        wifi_wait_connected(
            WIFI_CONNECT_TIMEOUT_MS);

    if (wifi_connected)
    {
        ESP_LOGI(TAG, "WiFi listo");
    }
    else
    {
        ESP_LOGW(TAG,
                 "WiFi no conectado, se continua sin red");

        wifi_scan_print();
    }

    // =====================================================
    // INICIALIZACIÓN DE HARDWARE
    // =====================================================

    i2c_manager_init();

    adc_manager_init();

    // =====================================================
    // CAUSA DE DESPERTAR
    // =====================================================

    esp_sleep_wakeup_cause_t cause =
        esp_sleep_get_wakeup_cause();

    ESP_LOGI(
        TAG,
        "Wakeup cause: %d",
        cause);

    // =====================================================
    // CONFIGURACIÓN DE WAKEUP
    // =====================================================

    esp_sleep_enable_ext0_wakeup(
        GPIO_NUM_33,
        0);

    // =====================================================
    // INICIALIZACIÓN RTC
    // =====================================================

    ds3231_init();

    ds3231_clear_alarm_flag();

    // =====================================================
    // CONFIGURACIÓN DE ALARMA RTC
    // =====================================================

    /**
     * Variable persistente utilizada para alternar
     * entre los instantes de despertar.
     */
    static RTC_DATA_ATTR int toggle = 0;

    if (toggle == 0)
    {
        ds3231_set_alarm_seconds(30);
        toggle = 1;
    }
    else
    {
        ds3231_set_alarm_seconds(0);
        toggle = 0;
    }

    // =====================================================
    // CREACIÓN DE COLAS
    // =====================================================

    queues_init();

    // =====================================================
    // CREACIÓN DE TAREAS
    // =====================================================

    BaseType_t res;

    res = xTaskCreate(
        task_sensores,
        "task_sensores",
        4096,
        NULL,
        5,
        &task_sensores_handle);

    if (res != pdPASS)
    {
        ESP_LOGE(TAG,
                 "Error creando task_sensores");
    }

    res = xTaskCreate(
        task_control,
        "task_control",
        4096,
        NULL,
        5,
        NULL);

    if (res != pdPASS)
    {
        ESP_LOGE(TAG,
                 "Error creando task_control");
    }

    res = xTaskCreate(
        task_actuadores,
        "task_actuadores",
        4096,
        NULL,
        5,
        NULL);

    if (res != pdPASS)
    {
        ESP_LOGE(TAG,
                 "Error creando task_actuadores");
    }

    res = xTaskCreate(
        task_logger,
        "task_logger",
        4096,
        NULL,
        5,
        NULL);

    if (res != pdPASS)
    {
        ESP_LOGE(TAG,
                 "Error creando task_logger");
    }

    res = xTaskCreate(
        task_mqtt,
        "task_mqtt",
        6144,
        NULL,
        5,
        NULL);

    if (res != pdPASS)
    {
        ESP_LOGE(TAG,
                 "Error creando task_mqtt");
    }

    res = xTaskCreate(
        task_display,
        "task_display",
        4096,
        NULL,
        4,
        NULL);

    if (res != pdPASS)
    {
        ESP_LOGE(TAG,
                 "Error creando task_display");
    }

    res = xTaskCreate(
        task_energia,
        "task_energia",
        4096,
        NULL,
        5,
        &task_energia_handle);

    if (res != pdPASS)
    {
        ESP_LOGE(TAG,
                 "Error creando task_energia");
    }

    ESP_LOGI(TAG, "Sistema listo");

    // =====================================================
    // INICIO DEL CICLO OPERATIVO
    // =====================================================

    xTaskNotifyGive(
        task_energia_handle);
}