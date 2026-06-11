/**
 * @file mqtt_manager.c
 * @brief Implementación del gestor de comunicación MQTT.
 *
 * Este módulo implementa la conexión con un broker MQTT
 * utilizando la biblioteca ESP-MQTT de ESP-IDF.
 *
 * La sincronización entre eventos MQTT y tareas de FreeRTOS
 * se realiza mediante Event Groups, permitiendo notificar:
 *
 * - Estado de conexión.
 * - Confirmación de publicaciones.
 * - Errores de comunicación.
 *
 * El módulo proporciona una interfaz desacoplada para que
 * otras tareas puedan publicar información sin interactuar
 * directamente con el cliente MQTT.
 */
#include "mqtt_manager.h"

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"

#include "esp_event.h"
#include "esp_log.h"
#include "mqtt_client.h"

#define MQTT_BROKER_URI "mqtt://192.168.0.110:1883"

/**
 * @brief Etiqueta utilizada por ESP_LOG.
 */
static const char *TAG = "mqtt_manager";

/**
 * @brief Instancia del cliente MQTT.
 */
static esp_mqtt_client_handle_t mqtt_client = NULL;

/**
 * @brief Grupo de eventos utilizado para sincronización MQTT.
 */
static EventGroupHandle_t mqtt_event_group = NULL;

/**
 * @brief Indica que el cliente MQTT está conectado.
 */
#define MQTT_CONNECTED_BIT BIT0

/**
 * @brief Indica que una publicación MQTT fue confirmada.
 */
#define MQTT_PUBLISHED_BIT BIT1

/**
 * @brief Indica que ocurrió un error MQTT.
 */
#define MQTT_ERROR_BIT BIT2

/**
 * @brief Manejador de eventos MQTT.
 *
 * Recibe los eventos generados por la biblioteca ESP-MQTT
 * y actualiza el grupo de eventos utilizado por FreeRTOS
 * para sincronizar las tareas de aplicación.
 *
 * Eventos gestionados:
 * - MQTT_EVENT_CONNECTED
 * - MQTT_EVENT_DISCONNECTED
 * - MQTT_EVENT_PUBLISHED
 * - MQTT_EVENT_ERROR
 *
 * @param handler_args Argumentos del manejador.
 * @param base Base del evento.
 * @param event_id Identificador del evento MQTT.
 * @param event_data Datos asociados al evento.
 */
static void mqtt_event_handler(void *handler_args,
                               esp_event_base_t base,
                               int32_t event_id,
                               void *event_data)
{
    esp_mqtt_event_handle_t event = event_data;

    switch ((esp_mqtt_event_id_t) event_id)
    {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "MQTT conectado");
            xEventGroupSetBits(mqtt_event_group, MQTT_CONNECTED_BIT);
            xEventGroupClearBits(mqtt_event_group, MQTT_ERROR_BIT);
            break;

        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGW(TAG, "MQTT desconectado");
            xEventGroupClearBits(mqtt_event_group, MQTT_CONNECTED_BIT);
            break;

        case MQTT_EVENT_PUBLISHED:
            ESP_LOGI(TAG, "MQTT publicado, msg_id=%d", event->msg_id);
            xEventGroupSetBits(mqtt_event_group, MQTT_PUBLISHED_BIT);
            break;

        case MQTT_EVENT_ERROR:
            ESP_LOGE(TAG, "Error MQTT");
            xEventGroupSetBits(mqtt_event_group, MQTT_ERROR_BIT);
            break;

        default:
            break;
    }
}

/**
 * @brief Inicializa y arranca el cliente MQTT.
 *
 * Crea el grupo de eventos necesario para la sincronización,
 * configura el cliente MQTT y comienza el proceso de conexión
 * con el broker.
 */
void mqtt_manager_start(void)
{
    if (mqtt_event_group == NULL)
    {
        mqtt_event_group = xEventGroupCreate();
    }

    if (mqtt_event_group == NULL)
    {
        ESP_LOGE(TAG, "No se pudo crear grupo de eventos MQTT");
        return;
    }

    xEventGroupClearBits(mqtt_event_group,
                         MQTT_CONNECTED_BIT | MQTT_PUBLISHED_BIT | MQTT_ERROR_BIT);

    if (mqtt_client != NULL)
    {
        return;
    }

    esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.uri = MQTT_BROKER_URI,
    };

    mqtt_client = esp_mqtt_client_init(&mqtt_cfg);
    if (mqtt_client == NULL)
    {
        ESP_LOGE(TAG, "No se pudo crear cliente MQTT");
        return;
    }

    esp_mqtt_client_register_event(
        mqtt_client,
        ESP_EVENT_ANY_ID,
        mqtt_event_handler,
        NULL);

    ESP_LOGI(TAG, "Conectando a broker MQTT: %s", MQTT_BROKER_URI);
    esp_mqtt_client_start(mqtt_client);
}

/**
 * @brief Espera el establecimiento de la conexión MQTT.
 *
 * Bloquea la tarea llamadora hasta que se reciba un evento
 * de conexión o se alcance el tiempo máximo de espera.
 *
 * @param timeout_ms Tiempo máximo de espera en milisegundos.
 *
 * @return true si se estableció la conexión.
 * @return false si ocurrió timeout o error.
 */
bool mqtt_wait_connected(uint32_t timeout_ms)
{
    if (mqtt_event_group == NULL)
    {
        return false;
    }

    EventBits_t bits = xEventGroupWaitBits(
        mqtt_event_group,
        MQTT_CONNECTED_BIT | MQTT_ERROR_BIT,
        pdFALSE,
        pdFALSE,
        pdMS_TO_TICKS(timeout_ms));

    return (bits & MQTT_CONNECTED_BIT) != 0;
}

/**
 * @brief Publica un mensaje en un tópico MQTT.
 *
 * Para QoS 1 o superior, la función espera la confirmación
 * de publicación utilizando un Event Group.
 *
 * @param topic Tópico MQTT.
 * @param payload Mensaje a publicar.
 * @param qos Nivel de QoS.
 * @param timeout_ms Tiempo máximo de espera.
 *
 * @return true si la publicación fue exitosa.
 * @return false si ocurrió un error o timeout.
 */
bool mqtt_publish(const char *topic, const char *payload, int qos, uint32_t timeout_ms)
{
    if (mqtt_client == NULL || mqtt_event_group == NULL)
    {
        return false;
    }

    xEventGroupClearBits(mqtt_event_group, MQTT_PUBLISHED_BIT | MQTT_ERROR_BIT);

    int msg_id = esp_mqtt_client_publish(
        mqtt_client,
        topic,
        payload,
        0,
        qos,
        0);

    if (msg_id < 0)
    {
        ESP_LOGE(TAG, "No se pudo publicar en MQTT");
        return false;
    }

    if (qos == 0)
    {
        return true;
    }

    EventBits_t bits = xEventGroupWaitBits(
        mqtt_event_group,
        MQTT_PUBLISHED_BIT | MQTT_ERROR_BIT,
        pdFALSE,
        pdFALSE,
        pdMS_TO_TICKS(timeout_ms));

    return (bits & MQTT_PUBLISHED_BIT) != 0;
}

/**
 * @brief Detiene el cliente MQTT.
 *
 * Finaliza la conexión con el broker MQTT, destruye la
 * instancia del cliente y limpia los bits de sincronización.
 */
void mqtt_manager_stop(void)
{
    if (mqtt_client != NULL)
    {
        esp_mqtt_client_stop(mqtt_client);
        esp_mqtt_client_destroy(mqtt_client);
        mqtt_client = NULL;
    }

    if (mqtt_event_group != NULL)
    {
        xEventGroupClearBits(mqtt_event_group,
                             MQTT_CONNECTED_BIT | MQTT_PUBLISHED_BIT | MQTT_ERROR_BIT);
    }
}
