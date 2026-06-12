/**
 * @file task_mqtt.c
 * @brief Implementación de la tarea de comunicación MQTT.
 *
 * Esta tarea recibe mediciones provenientes del sistema,
 * genera mensajes JSON y los publica en un broker MQTT.
 *
 * Funcionalidades principales:
 * - Publicación de datos mediante MQTT.
 * - Generación de payloads JSON.
 * - Reintento automático de mensajes pendientes.
 * - Almacenamiento local cuando no existe conectividad.
 * - Recuperación de mensajes almacenados en SD.
 *
 * Arquitectura:
 *
 * queue_mqtt
 *       |
 *       v
 *   task_mqtt
 *       |
 *       +------> MQTT Broker
 *       |
 *       +------> Archivo de pendientes (SD)
 *
 * Estrategia de tolerancia a fallos:
 *
 * Si el broker MQTT no está disponible:
 * - El dato no se pierde.
 * - Se almacena en la tarjeta SD.
 * - Se reenvía automáticamente cuando la conexión se recupera.
 *
 * Mecanismos FreeRTOS utilizados:
 * - Colas.
 * - Notificaciones entre tareas.
 *
 * @author
 * Fernando Plazas Trujillo
 * Isabella Ordoñez
 * Juan Daniel Constain
 */

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "drivers/sd_card.h"
#include "mqtt/mqtt_manager.h"
#include "queues.h"
#include "utils/data_types.h"

/**
 * @brief Tópico MQTT utilizado para publicar mediciones.
 */
#define MQTT_TOPIC "fermentador/espInterior/datos"

/**
 * @brief Tiempo máximo de espera para conexión MQTT.
 */
#define MQTT_CONNECT_TIMEOUT_MS 8000

/**
 * @brief Tiempo máximo de espera para publicación MQTT.
 */
#define MQTT_PUBLISH_TIMEOUT_MS 5000

/**
 * @brief Tamaño máximo permitido para payloads MQTT.
 */
#define MQTT_PENDING_PAYLOAD_SIZE 320

/**
 * @brief Handle de la tarea de energía.
 *
 * Se utiliza para notificar la finalización
 * del proceso de publicación.
 */
extern TaskHandle_t task_energia_handle;

/**
 * @brief Guarda un payload MQTT pendiente en la SD.
 *
 * Se utiliza cuando el broker MQTT no está disponible
 * o cuando ocurre un error durante la publicación.
 *
 * @param payload Cadena JSON a almacenar.
 */
static void save_pending_payload(const char *payload)
{
    if (sd_card_append_pending_mqtt(payload) == ESP_OK)
    {
        printf("Payload MQTT guardado en pendientes\n");
    }
    else
    {
        printf("Error guardando payload MQTT pendiente\n");
    }
}

/**
 * @brief Reenvía mensajes MQTT almacenados en la SD.
 *
 * Lee todos los mensajes pendientes almacenados
 * localmente y los publica nuevamente en el broker.
 *
 * Si todos los mensajes son enviados correctamente,
 * el archivo de pendientes es eliminado.
 *
 * @return true si todos los mensajes fueron reenviados.
 * @return false si ocurrió algún error.
 */
static bool resend_pending_mqtt_messages(void)
{
    char pending_payload[MQTT_PENDING_PAYLOAD_SIZE];
    FILE *pending_file;

    if (!sd_card_pending_mqtt_exists())
    {
        return true;
    }

    printf("Reenviando mensajes MQTT pendientes desde SD\n");

    pending_file = sd_card_open_pending_mqtt_read();

    if (pending_file == NULL)
    {
        return false;
    }

    while (sd_card_read_pending_mqtt_line(
               pending_file,
               pending_payload,
               sizeof(pending_payload)) != NULL)
    {
        pending_payload[strcspn(
            pending_payload,
            "\r\n")] = '\0';

        if (pending_payload[0] == '\0')
        {
            continue;
        }

        printf("MQTT pendiente -> %s\n", pending_payload);

        if (!mqtt_publish(
                MQTT_TOPIC,
                pending_payload,
                1,
                MQTT_PUBLISH_TIMEOUT_MS))
        {
            printf("Error reenviando payload MQTT pendiente\n");

            sd_card_close_pending_mqtt(
                pending_file);

            return false;
        }
    }

    sd_card_close_pending_mqtt(
        pending_file);

    if (sd_card_delete_pending_mqtt() != ESP_OK)
    {
        printf("Mensajes pendientes reenviados, pero no se pudo eliminar el archivo\n");
        return false;
    }

    printf("Mensajes MQTT pendientes reenviados correctamente\n");

    return true;
}

/**
 * @brief Construye un payload JSON a partir de una medición.
 *
 * Convierte una estructura sensor_data_t en un mensaje
 * JSON compatible con el broker MQTT y los servicios
 * de monitoreo externos.
 *
 * @param data Datos del sensor.
 * @param payload Buffer de salida.
 * @param payload_size Tamaño del buffer.
 */
static void build_payload(
    const sensor_data_t *data,
    char *payload,
    size_t payload_size)
{
    snprintf(
        payload,
        payload_size,
        "{\"timestamp\":\"%04d-%02d-%02d %02d:%02d:%02d\","
        "\"temperatura\":%.2f,\"ph\":%.2f,\"co2\":%.4f,"
        "\"co2_raw\":%.4f,\"voltaje\":%.2f,\"corriente\":%.2f}",
        data->datetime.year,
        data->datetime.month,
        data->datetime.day,
        data->datetime.hours,
        data->datetime.minutes,
        data->datetime.seconds,
        data->temperatura,
        data->ph,
        data->co2,
        data->co2_raw,
        data->voltaje,
        data->corriente);
}

/**
 * @brief Tarea principal de comunicación MQTT.
 *
 * Espera nuevas mediciones desde queue_mqtt,
 * genera el payload JSON correspondiente y
 * realiza la publicación en el broker MQTT.
 *
 * En caso de fallo:
 * - El dato se almacena en SD.
 * - Se reintentará posteriormente.
 *
 * Una vez finalizado el proceso, se notifica
 * a task_energia para continuar el ciclo operativo.
 *
 * @param pvParameters Parámetros de la tarea (no utilizados).
 */
void task_mqtt(void *pvParameters)
{
    sensor_data_t data;
    char payload[MQTT_PENDING_PAYLOAD_SIZE];

    while (1)
    {
        if (xQueueReceive(
                queue_mqtt,
                &data,
                portMAX_DELAY))
        {
            build_payload(
                &data,
                payload,
                sizeof(payload));

            printf(
                "MQTT payload -> %s\n",
                payload);

            mqtt_manager_start();

            if (!mqtt_wait_connected(
                    MQTT_CONNECT_TIMEOUT_MS))
            {
                printf("MQTT no conectado\n");

                save_pending_payload(payload);

                mqtt_manager_stop();

                xTaskNotifyGive(
                    task_energia_handle);

                continue;
            }

            if (!resend_pending_mqtt_messages())
            {
                printf("No se pudieron reenviar todos los pendientes MQTT\n");

                save_pending_payload(payload);

                mqtt_manager_stop();

                xTaskNotifyGive(
                    task_energia_handle);

                continue;
            }

            if (mqtt_publish(
                    MQTT_TOPIC,
                    payload,
                    1,
                    MQTT_PUBLISH_TIMEOUT_MS))
            {
                printf("Dato publicado por MQTT\n");
            }
            else
            {
                printf("Error publicando por MQTT");

                save_pending_payload(payload);
            }

            mqtt_manager_stop();

            xTaskNotifyGive(
                task_energia_handle);
        }
    }
}