#include "mqtt_manager.h"

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"

#include "esp_event.h"
#include "esp_log.h"
#include "mqtt_client.h"

#define MQTT_BROKER_URI "mqtt://10.51.221.153:1883"

static const char *TAG = "mqtt_manager";

static esp_mqtt_client_handle_t mqtt_client = NULL;
static EventGroupHandle_t mqtt_event_group = NULL;

#define MQTT_CONNECTED_BIT BIT0
#define MQTT_PUBLISHED_BIT BIT1
#define MQTT_ERROR_BIT     BIT2

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
