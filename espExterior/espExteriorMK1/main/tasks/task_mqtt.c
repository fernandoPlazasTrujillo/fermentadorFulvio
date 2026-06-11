/**
 * @file task_mqtt.c
 * @brief Tarea encargada de la transmisión MQTT de datos ambientales.
 *
 * Esta tarea recibe datos desde una cola FreeRTOS, genera un
 * mensaje JSON y lo publica en un broker MQTT.
 *
 * Para aumentar la confiabilidad del sistema, los mensajes que
 * no pueden ser enviados debido a fallos de conectividad son
 * almacenados temporalmente en la tarjeta SD y reenviados
 * automáticamente cuando la conexión se restablece.
 *
 * Responsabilidades:
 * - Recibir datos desde la cola MQTT.
 * - Construir el payload JSON.
 * - Gestionar la conexión MQTT.
 * - Publicar datos en el broker.
 * - Almacenar mensajes pendientes en SD.
 * - Reenviar mensajes pendientes.
 * - Notificar la finalización del ciclo.
 *
 * Aspectos de FreeRTOS utilizados:
 * - Queue para recepción de datos.
 * - Task Notification para sincronización.
 * - Eliminación dinámica de tarea mediante vTaskDelete().
 *
 * Estrategia de tolerancia a fallos:
 * - Si MQTT falla, los datos se almacenan en SD.
 * - Al recuperar la conexión se reenvían los mensajes pendientes.
 * - Se evita la pérdida de información durante cortes de red.
 *
 * @authors
 * - Fernando Plazas
 * - Isabella Ordoñez
 * - Juan Daniel Constain
 */
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "common.h"
#include "mqtt/mqtt_manager.h"
#include "queues.h"

#include "sd_card.h"

/**
 * @brief Tópico MQTT utilizado para la publicación.
 */
#define MQTT_TOPIC "fermentador/espExterior/datos"

/**
 * @brief Tiempo máximo de espera para conexión MQTT.
 */
#define MQTT_CONNECT_TIMEOUT_MS 8000

/**
 * @brief Tiempo máximo de espera para confirmación de publicación.
 */
#define MQTT_PUBLISH_TIMEOUT_MS 5000

/**
 * @brief Tamaño máximo del buffer utilizado para mensajes pendientes.
 */
#define MQTT_PENDING_PAYLOAD_SIZE 256

/**
 * @brief Handle de la tarea principal.
 *
 * Utilizado para notificar la finalización del proceso MQTT.
 */
extern TaskHandle_t main_task_handle;

/**
 * @brief Guarda un payload MQTT pendiente en la tarjeta SD.
 *
 * Esta función es utilizada cuando no es posible completar
 * una transmisión MQTT debido a problemas de conectividad.
 *
 * @param payload Mensaje MQTT a almacenar.
 */
static void save_pending_payload(const char *payload)
{
    if (sd_append_pending_mqtt(payload) == 0)
    {
        printf("Payload MQTT guardado en pendientes\n");
    }
    else
    {
        printf("Error guardando payload MQTT pendiente\n");
    }
}

/**
 * @brief Reenvía mensajes MQTT pendientes almacenados en SD.
 *
 * Recorre el archivo de mensajes pendientes y publica cada
 * registro en el broker MQTT.
 *
 * Si todos los mensajes son enviados correctamente,
 * el archivo de pendientes es eliminado.
 *
 * @return true si todos los mensajes fueron reenviados.
 * @return false si ocurrió algún error durante el proceso.
 */
static bool resend_pending_mqtt_messages(void)
{
    char pending_payload[MQTT_PENDING_PAYLOAD_SIZE];
    FILE *pending_file;

    if (!sd_pending_mqtt_exists())
    {
        return true;
    }

    printf("Reenviando mensajes MQTT pendientes desde SD\n");

    pending_file = sd_open_pending_mqtt_read();
    if (pending_file == NULL)
    {
        return false;
    }

    while (sd_read_pending_mqtt_line(pending_file,
                                     pending_payload,
                                     sizeof(pending_payload)) != NULL)
    {
        pending_payload[strcspn(pending_payload, "\r\n")] = '\0';

        if (pending_payload[0] == '\0')
        {
            continue;
        }

        printf("MQTT pendiente -> %s\n", pending_payload);

        if (!mqtt_publish(MQTT_TOPIC,
                          pending_payload,
                          1,
                          MQTT_PUBLISH_TIMEOUT_MS))
        {
            printf("Error reenviando payload MQTT pendiente\n");
            sd_close_pending_mqtt(pending_file);
            return false;
        }
    }

    sd_close_pending_mqtt(pending_file);

    if (sd_delete_pending_mqtt() != 0)
    {
        printf("Mensajes pendientes reenviados, pero no se pudo eliminar test.txt\n");
        return false;
    }

    printf("Mensajes MQTT pendientes reenviados correctamente\n");

    return true;
}

/**
 * @brief Tarea responsable de la comunicación MQTT.
 *
 * Flujo de ejecución:
 *
 * 1. Espera datos desde la cola MQTT.
 * 2. Construye el mensaje JSON.
 * 3. Inicia el cliente MQTT.
 * 4. Espera conexión con el broker.
 * 5. Reenvía mensajes pendientes.
 * 6. Publica el mensaje actual.
 * 7. Notifica a la tarea principal.
 * 8. Finaliza su ejecución.
 *
 * En caso de pérdida de conectividad:
 * - El mensaje actual se almacena en SD.
 * - La tarea termina sin perder información.
 *
 * @param pvParameters Parámetros de la tarea (no utilizados).
 */
void task_mqtt(void *pvParameters)
{
    datos_ambiente_t datos;
    char payload[192];

    vTaskDelay(pdMS_TO_TICKS(3000));

    if (xQueueReceive(cola_mqtt, &datos, portMAX_DELAY))
    {
        snprintf(payload, sizeof(payload),
            "{\"timestamp\":\"%04d-%02d-%02d %02d:%02d:%02d\",\"temp\":%.2f,\"hum\":%.2f}",
            datos.anio,
            datos.mes,
            datos.dia,
            datos.hora,
            datos.minuto,
            datos.segundo,
            datos.temperatura,
            datos.humedad);

        printf("MQTT payload -> %s\n", payload);

        mqtt_manager_start();

        if (!mqtt_wait_connected(MQTT_CONNECT_TIMEOUT_MS))
        {
            printf("MQTT no conectado\n");

            save_pending_payload(payload);

            mqtt_manager_stop();

            xTaskNotifyGive(main_task_handle);

            vTaskDelete(NULL);
        }

        if (!resend_pending_mqtt_messages())
        {
            printf("No se pudieron reenviar todos los pendientes MQTT\n");
            save_pending_payload(payload);

            mqtt_manager_stop();
            xTaskNotifyGive(main_task_handle);
            vTaskDelete(NULL);
        }

        if (mqtt_publish(MQTT_TOPIC, payload, 1, MQTT_PUBLISH_TIMEOUT_MS))
        {
            printf("Dato publicado por MQTT\n");
        }
        else
        {
            printf("Error publicando por MQTT\n");

            save_pending_payload(payload);
        }
    }

    mqtt_manager_stop();
    xTaskNotifyGive(main_task_handle);
    vTaskDelete(NULL);
}
