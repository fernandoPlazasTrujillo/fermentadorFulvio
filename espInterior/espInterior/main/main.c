/**
 * @file main.c
 * @brief Punto de entrada del sistema.
 * 
 * Inicializa hardware, configura el RTC, crea las tareas
 * y gestiona el flujo principal del sistema.
 * 
 * Coordina:
 * - Inicialización de HAL
 * - Configuración de wakeup
 * - Creación de tareas
 * - Inicio del ciclo de ejecución
 * 
 * @author
 * Fernando Plazas Trujillo
 * Isabella Ordoñez
 * Juan Daniel Constain
 */

#include <stdio.h>

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

// Queues
#include "queues.h"

// =====================
// HANDLES
// =====================

/** @brief Handle de la tarea de sensores */
TaskHandle_t task_sensores_handle = NULL;

/** @brief Handle de la tarea de energía */
TaskHandle_t task_energia_handle  = NULL;

static const char *TAG = "MAIN";

// =====================
// MAIN
// =====================

/**
 * @brief Función principal del sistema.
 */
void app_main(void)
{
    ESP_LOGI(TAG, "Iniciando sistema...");

    // =====================
    // INIT HARDWARE
    // =====================

    i2c_manager_init();
    adc_manager_init();

    // =====================
    // WAKEUP CAUSE
    // =====================

    esp_sleep_wakeup_cause_t cause = esp_sleep_get_wakeup_cause();
    ESP_LOGI(TAG, "Wakeup cause: %d", cause);

    // =====================
    // CONFIG WAKEUP
    // =====================

    esp_sleep_enable_ext0_wakeup(GPIO_NUM_33, 0);

    // =====================
    // RTC
    // =====================

    ds3231_init();
    ds3231_clear_alarm_flag();

    // =====================
    // ALARMA
    // =====================

    static RTC_DATA_ATTR int toggle = 0;

    if (toggle == 0) {
        ds3231_set_alarm_seconds(30);
        toggle = 1;
    } else {
        ds3231_set_alarm_seconds(0);
        toggle = 0;
    }

    // =====================
    // QUEUES
    // =====================

    queues_init();

    // =====================
    // TASKS
    // =====================

    BaseType_t res;

    res = xTaskCreate(task_sensores, "task_sensores", 4096, NULL, 5, &task_sensores_handle);
    if (res != pdPASS) ESP_LOGE(TAG, "Error creando task_sensores");

    res = xTaskCreate(task_control, "task_control", 4096, NULL, 5, NULL);
    if (res != pdPASS) ESP_LOGE(TAG, "Error creando task_control");

    res = xTaskCreate(task_actuadores, "task_actuadores", 4096, NULL, 5, NULL);
    if (res != pdPASS) ESP_LOGE(TAG, "Error creando task_actuadores");

    res = xTaskCreate(task_logger, "task_logger", 4096, NULL, 5, NULL);
    if (res != pdPASS) ESP_LOGE(TAG, "Error creando task_logger");

    res = xTaskCreate(task_energia, "task_energia", 4096, NULL, 5, &task_energia_handle);
    if (res != pdPASS) ESP_LOGE(TAG, "Error creando task_energia");

    ESP_LOGI(TAG, "Sistema listo");

    // =====================
    // INICIO DEL CICLO
    // =====================

    xTaskNotifyGive(task_energia_handle);
}