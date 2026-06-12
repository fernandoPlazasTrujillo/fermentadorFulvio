/**
 * @file task_sensores.c
 * @brief Implementación de la tarea de adquisición de sensores.
 *
 * Esta tarea es responsable de realizar la lectura de los sensores
 * del sistema de fermentación, procesar las mediciones obtenidas
 * y distribuir los datos a las diferentes tareas mediante colas
 * FreeRTOS.
 *
 * Sensores gestionados:
 * - DS18B20 (temperatura).
 * - MQ-135 (CO2).
 * - Sensor de pH.
 * - RTC DS3231.
 *
 * Funciones principales:
 * - Adquisición de datos.
 * - Filtrado de señales.
 * - Cálculo de referencia (baseline).
 * - Obtención de fecha y hora.
 * - Distribución de información mediante colas.
 *
 * Arquitectura:
 *
 *        Sensores
 *            |
 *            v
 *     task_sensores
 *            |
 *     +------+------+------+------+
 *     |      |      |      |      |
 *     v      v      v      v      v
 * control logger mqtt display
 *
 * Mecanismos FreeRTOS utilizados:
 * - Notificaciones de tareas.
 * - Colas de comunicación.
 *
 * Persistencia:
 * - Uso de RTC_DATA_ATTR para conservar variables
 *   durante ciclos de Deep Sleep.
 *
 * @author
 * Fernando Plazas Trujillo
 * Isabella Ordoñez
 * Juan Daniel Constain
 */

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"

#include "utils/data_types.h"
#include "drivers/rtc_ds3231.h"
#include "drivers/mq135.h"
#include "drivers/ph_sensor.h"
#include "drivers/ds18b20.h"
#include "queues.h"

#include "wifi/wifi_manager.h"
#include "mqtt/mqtt_manager.h"
#include "drivers/sd_card.h"

/**
 * @brief Etiqueta utilizada para mensajes de depuración.
 */
static const char *TAG = "SENSORES";

// =====================================================
// PERSISTENCIA EN DEEP SLEEP
// =====================================================

/**
 * @brief Valor de referencia del sensor MQ135.
 *
 * Se conserva durante ciclos de Deep Sleep mediante
 * memoria RTC.
 */
RTC_DATA_ATTR static float baseline = 0;

/**
 * @brief Indica si el baseline ya fue inicializado.
 */
RTC_DATA_ATTR static int baseline_initialized = 0;

// =====================================================
// FILTRO EXPONENCIAL
// =====================================================

/**
 * @brief Constante del filtro exponencial.
 */
#define ALPHA 0.2f

/**
 * @brief Valor filtrado actual del sensor MQ135.
 */
static float filtered = 0;

/**
 * @brief Tarea principal de adquisición de sensores.
 *
 * Permanece bloqueada esperando una notificación de
 * activación proveniente de task_energia.
 *
 * Una vez activada:
 * - Lee todos los sensores.
 * - Procesa las mediciones.
 * - Obtiene fecha y hora.
 * - Construye estructuras compartidas.
 * - Distribuye los datos mediante colas FreeRTOS.
 *
 * @param pvParameters Parámetros de la tarea (no utilizados).
 */
void task_sensores(void *pvParameters)
{
    sensor_data_t data;

    // =====================================================
    // INICIALIZACIÓN DE DRIVERS
    // =====================================================

    mq135_init();

    ph_init();

    ds18b20_init(GPIO_NUM_4);

    while (1)
    {
        // =====================================================
        // ESPERAR ACTIVACIÓN
        // =====================================================

        ulTaskNotifyTake(
            pdTRUE,
            portMAX_DELAY);

        ESP_LOGI(TAG, "Leyendo sensores...");

        // =====================================================
        // SENSOR MQ135
        // =====================================================

        float raw_voltage =
            mq135_read();

        int raw_adc =
            mq135_read_raw();

        ESP_LOGI(
            TAG,
            "MQ135 ADC RAW: %d",
            raw_adc);

        /**
         * Aplicar filtro exponencial para reducir
         * ruido en la señal.
         */
        filtered =
            ALPHA * raw_voltage +
            (1.0f - ALPHA) * filtered;

        /**
         * Inicializar referencia base del sensor.
         */
        if (!baseline_initialized)
        {
            baseline = filtered;

            baseline_initialized = 1;

            ESP_LOGI(
                TAG,
                "Baseline inicial: %.4f",
                baseline);
        }

        data.co2_raw = raw_voltage;

        data.co2 =
            filtered - baseline;

        // =====================================================
        // SENSOR DE PH
        // =====================================================

        data.ph =
            ph_read();

        // =====================================================
        // TEMPERATURA DS18B20
        // =====================================================

        float temp =
            ds18b20_read_temperature();

        if (temp == -127.0f)
        {
            ESP_LOGW(
                TAG,
                "Error leyendo DS18B20");

            temp = 25.0f;
        }

        data.temperatura = temp;

        // =====================================================
        // VARIABLES DE PRUEBA
        // =====================================================

        data.voltaje = 5.0f;

        data.corriente = 0.5f;

        // =====================================================
        // RTC
        // =====================================================

        ds3231_get_datetime(
            &data.datetime);

        printf(
            "RTC -> %02d:%02d:%02d\n",
            data.datetime.hours,
            data.datetime.minutes,
            data.datetime.seconds);

        // =====================================================
        // ENVÍO A TASK CONTROL
        // =====================================================

        xQueueSend(
            queue_sensores,
            &data,
            portMAX_DELAY);

        // =====================================================
        // ENVÍO A LOGGER
        // =====================================================

        xQueueSend(
            queue_logger,
            &data,
            portMAX_DELAY);

        // =====================================================
        // ENVÍO A MQTT
        // =====================================================

        xQueueSend(
            queue_mqtt,
            &data,
            portMAX_DELAY);

        // =====================================================
        // DATOS DE DISPLAY
        // =====================================================

        display_data_t display;

        display.temperatura =
            data.temperatura;

        display.ph =
            data.ph;

        display.co2 =
            data.co2;

        display.wifi_ok =
            wifi_is_connected();

        display.mqtt_ok =
            mqtt_is_connected();

        display.sd_ok =
            sd_card_is_mounted();

        display.datetime =
            data.datetime;

        xQueueSend(
            queue_display,
            &display,
            0);

        ESP_LOGI(
            TAG,
            "Datos enviados");
    }
}