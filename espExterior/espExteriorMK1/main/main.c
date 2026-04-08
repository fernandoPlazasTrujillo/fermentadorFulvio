/**
 * @file main.c
 * @brief Punto de entrada del sistema ESP32 para monitoreo ambiental.
 *
 * Este módulo inicializa el sistema, configura periféricos y coordina
 * la ejecución de tareas bajo FreeRTOS.
 *
 * Funcionalidades principales:
 * - Inicialización del bus I2C
 * - Inicialización de la tarjeta SD
 * - Creación de colas de comunicación
 * - Creación de tareas del sistema
 * - Configuración del RTC DS3231 para wakeup periódico
 * - Gestión de bajo consumo mediante Deep Sleep
 *
 * Arquitectura:
 * - task_sensores → adquisición de datos
 * - task_logger → almacenamiento en SD
 * - task_display → visualización en LCD
 *
 * Flujo general:
 * 1. Inicialización del sistema
 * 2. Ejecución de tareas
 * 3. Espera sincronización (display)
 * 4. Configuración de alarma RTC
 * 5. Entrada en Deep Sleep
 *
 * @note El sistema funciona en ciclos:
 * medir → procesar → mostrar → guardar → dormir
 *
 * @warning El pin SQW del DS3231 debe estar correctamente conectado
 * para permitir el wakeup desde Deep Sleep.
 *
 * @authors
 * - Fernando Plazas
 * - Isabella Ordoñez
 * - Juan Daniel Constain
 */

#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/i2c.h"
#include "driver/gpio.h"
#include "esp_sleep.h"

#include "queues.h"
#include "common.h"
#include "ds3231.h"
#include "sd_card.h"

// ==========================
// CONFIGURACIÓN I2C
// ==========================

#define I2C_MASTER_SCL_IO 22        /**< Pin SCL */
#define I2C_MASTER_SDA_IO 21        /**< Pin SDA */
#define I2C_MASTER_FREQ_HZ 100000   /**< Frecuencia I2C */

// ==========================
// CONFIGURACIÓN RTC
// ==========================

#define RTC_INT_GPIO GPIO_NUM_27    /**< Pin SQW del DS3231 */

// ==========================
// COLAS DEL SISTEMA
// ==========================

QueueHandle_t cola_eventos;  /**< Cola de eventos del sistema */
QueueHandle_t cola_datos;    /**< Cola de datos para logging */
QueueHandle_t cola_display;  /**< Cola de datos para visualización */

// ==========================
// HANDLE DE TAREA PRINCIPAL
// ==========================

/**
 * @brief Handle de la tarea principal
 *
 * Permite sincronización mediante notificaciones desde otras tareas.
 */
TaskHandle_t main_task_handle = NULL;

// ==========================
// DECLARACIÓN DE TAREAS
// ==========================

extern void task_input(void *pvParameters);
extern void task_sensores(void *pvParameters);
extern void task_logger(void *pvParameters);
extern void task_display(void *pvParameters);

// ==========================
// INICIALIZACIÓN I2C
// ==========================

/**
 * @brief Inicializa el bus I2C en modo maestro
 *
 * Configura los pines SDA y SCL y establece la frecuencia del bus.
 *
 * @note Necesario para comunicación con el RTC DS3231.
 */
void i2c_init()
{
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ
    };

    i2c_param_config(I2C_NUM_0, &conf);
    i2c_driver_install(I2C_NUM_0, conf.mode, 0, 0, 0);
}

// ==========================
// FUNCIÓN PRINCIPAL
// ==========================

void app_main(void)
{
    printf("Sistema iniciado\n");

    /**
     * @brief Obtener handle de la tarea principal
     */
    main_task_handle = xTaskGetCurrentTaskHandle();

    /**
     * @brief Inicializar I2C
     */
    i2c_init();

    // ==========================
    // DETECCIÓN DE WAKEUP
    // ==========================

    /**
     * @brief Determina la causa de activación desde Deep Sleep
     */
    esp_sleep_wakeup_cause_t cause = esp_sleep_get_wakeup_cause();

    if (cause == ESP_SLEEP_WAKEUP_EXT0)
    {
        printf("Wakeup por RTC\n");
    }

    // ==========================
    // INICIALIZACIÓN SD
    // ==========================

    if (sd_init() != 0)
    {
        printf("SD no inicializada\n");
    }

    // ==========================
    // CONFIGURACIÓN RTC
    // ==========================

    /**
     * @brief Limpia el flag de alarma del RTC
     *
     * Evita falsas interrupciones al iniciar el sistema.
     */
    ds3231_clear_alarm_flag();

    // ==========================
    // CREACIÓN DE COLAS
    // ==========================

    cola_eventos  = xQueueCreate(10, sizeof(evento_t));
    cola_datos    = xQueueCreate(10, sizeof(datos_ambiente_t));
    cola_display  = xQueueCreate(10, sizeof(datos_ambiente_t));

    // ==========================
    // CREACIÓN DE TAREAS
    // ==========================

    xTaskCreate(task_input,    "task_input",    2048, NULL, 5, NULL);
    xTaskCreate(task_sensores, "task_sensores", 4096, NULL, 5, NULL);
    xTaskCreate(task_logger,   "task_logger",   4096, NULL, 5, NULL);
    xTaskCreate(task_display,  "task_display",  4096, NULL, 5, NULL);

    // ==========================
    // SINCRONIZACIÓN
    // ==========================

    /**
     * @brief Espera a que la tarea de display finalice
     *
     * Se utiliza notificación de tarea para sincronizar el flujo.
     */
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

    // ==========================
    // CONFIGURACIÓN ALARMA RTC
    // ==========================

    printf("Configurando alarma RTC...\n");

    ds3231_set_alarm_every_minute();

    /**
     * @brief Limpia el flag antes de dormir
     */
    ds3231_clear_alarm_flag();

    /**
     * @brief Retardo para estabilización del pin SQW
     */
    vTaskDelay(pdMS_TO_TICKS(20));

    // ==========================
    // CONFIGURACIÓN GPIO WAKEUP
    // ==========================

    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << RTC_INT_GPIO),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .intr_type = GPIO_INTR_DISABLE
    };

    gpio_config(&io_conf);

    /**
     * @brief Lectura del nivel del pin SQW (debug)
     */
    int level = gpio_get_level(RTC_INT_GPIO);
    printf("SQW antes de dormir: %d\n", level);

    // ==========================
    // MODO DE BAJO CONSUMO
    // ==========================

    printf("Entrando en deep sleep...\n");

    /**
     * @brief Configura wakeup por señal externa (RTC)
     */
    esp_sleep_enable_ext0_wakeup(RTC_INT_GPIO, 0);

    /**
     * @brief Entrada en Deep Sleep
     */
    esp_deep_sleep_start();
}