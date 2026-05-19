#include "wifi_manager.h"

#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"

#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_wifi.h"

#include "nvs_flash.h"

#define WIFI_SSID      "PLAZATRUJILLO"
#define WIFI_PASS      "47234189"
#define WIFI_MAX_RETRIES 4
#define WIFI_SCAN_MAX_AP 20

static const char *TAG = "wifi_manager";
static int retry_count = 0;

/**
 * @brief Event group para manejar estados del WiFi.
 */
static EventGroupHandle_t wifi_event_group;

/**
 * @brief Bit que indica conexión WiFi exitosa.
 */
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

/**
 * @brief Handler de eventos WiFi e IP.
 */
static void wifi_event_handler(
    void *arg,
    esp_event_base_t event_base,
    int32_t event_id,
    void *event_data)
{
    /**
     * WiFi iniciado.
     */
    if (event_base == WIFI_EVENT &&
        event_id == WIFI_EVENT_STA_START)
    {
        ESP_LOGI(TAG, "Conectando al WiFi...");
        retry_count = 0;
        xEventGroupClearBits(
            wifi_event_group,
            WIFI_CONNECTED_BIT | WIFI_FAIL_BIT);
        esp_wifi_connect();
    }

    /**
     * WiFi desconectado.
     */
    else if (event_base == WIFI_EVENT &&
             event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        wifi_event_sta_disconnected_t *event =
            (wifi_event_sta_disconnected_t *) event_data;

        xEventGroupClearBits(
            wifi_event_group,
            WIFI_CONNECTED_BIT);

        if (retry_count < WIFI_MAX_RETRIES)
        {
            retry_count++;
            ESP_LOGW(TAG,
                     "WiFi desconectado. Razon=%d. Reintento %d/%d...",
                     event->reason,
                     retry_count,
                     WIFI_MAX_RETRIES);
            esp_wifi_connect();
        }
        else
        {
            ESP_LOGW(TAG,
                     "WiFi desconectado. Razon=%d. Sin mas reintentos.",
                     event->reason);
            xEventGroupSetBits(
                wifi_event_group,
                WIFI_FAIL_BIT);
        }
    }

    /**
     * IP obtenida correctamente.
     */
    else if (event_base == IP_EVENT &&
             event_id == IP_EVENT_STA_GOT_IP)
    {
        ip_event_got_ip_t *event =
            (ip_event_got_ip_t *) event_data;

        ESP_LOGI(TAG,
                 "WiFi conectado");

        ESP_LOGI(TAG,
                 "IP obtenida: " IPSTR,
                 IP2STR(&event->ip_info.ip));

        xEventGroupSetBits(
            wifi_event_group,
            WIFI_CONNECTED_BIT);
        xEventGroupClearBits(
            wifi_event_group,
            WIFI_FAIL_BIT);
        retry_count = 0;
    }
}

/**
 * @brief Inicializa y conecta el ESP32 al WiFi.
 */
void wifi_init(void)
{
    /**
     * Crear event group.
     */
    wifi_event_group = xEventGroupCreate();

    /**
     * Inicializar NVS.
     */
    esp_err_t ret = nvs_flash_init();

    if (ret == ESP_ERR_NVS_NO_FREE_PAGES ||
        ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());

        ret = nvs_flash_init();
    }

    ESP_ERROR_CHECK(ret);

    /**
     * Inicializar TCP/IP stack.
     */
    ESP_ERROR_CHECK(esp_netif_init());

    /**
     * Crear event loop.
     */
    ESP_ERROR_CHECK(
        esp_event_loop_create_default());

    /**
     * Crear interfaz WiFi station.
     */
    esp_netif_create_default_wifi_sta();

    /**
     * Configuración WiFi por defecto.
     */
    wifi_init_config_t cfg =
        WIFI_INIT_CONFIG_DEFAULT();

    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    /**
     * Registrar handlers de eventos.
     */
    ESP_ERROR_CHECK(
        esp_event_handler_instance_register(
            WIFI_EVENT,
            ESP_EVENT_ANY_ID,
            &wifi_event_handler,
            NULL,
            NULL));

    ESP_ERROR_CHECK(
        esp_event_handler_instance_register(
            IP_EVENT,
            IP_EVENT_STA_GOT_IP,
            &wifi_event_handler,
            NULL,
            NULL));

    /**
     * Configuración de credenciales WiFi.
     */
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASS,
        },
    };

    /**
     * Configurar modo station.
     */
    ESP_ERROR_CHECK(
        esp_wifi_set_mode(WIFI_MODE_STA));

    /**
     * Aplicar configuración WiFi.
     */
    ESP_ERROR_CHECK(
        esp_wifi_set_config(
            WIFI_IF_STA,
            &wifi_config));

    /**
     * Iniciar WiFi.
     */
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG,
             "Inicialización WiFi completada");
}

bool wifi_wait_connected(uint32_t timeout_ms)
{
    if (wifi_event_group == NULL)
    {
        return false;
    }

    EventBits_t bits = xEventGroupWaitBits(
        wifi_event_group,
        WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
        pdFALSE,
        pdFALSE,
        pdMS_TO_TICKS(timeout_ms));

    return (bits & WIFI_CONNECTED_BIT) != 0;
}

static const char *auth_mode_name(wifi_auth_mode_t authmode)
{
    switch (authmode)
    {
        case WIFI_AUTH_OPEN:
            return "OPEN";
        case WIFI_AUTH_WEP:
            return "WEP";
        case WIFI_AUTH_WPA_PSK:
            return "WPA";
        case WIFI_AUTH_WPA2_PSK:
            return "WPA2";
        case WIFI_AUTH_WPA_WPA2_PSK:
            return "WPA/WPA2";
        case WIFI_AUTH_WPA2_ENTERPRISE:
            return "WPA2_ENT";
        case WIFI_AUTH_WPA3_PSK:
            return "WPA3";
        case WIFI_AUTH_WPA2_WPA3_PSK:
            return "WPA2/WPA3";
        default:
            return "UNKNOWN";
    }
}

void wifi_scan_print(void)
{
    wifi_ap_record_t ap_records[WIFI_SCAN_MAX_AP];
    uint16_t ap_count = WIFI_SCAN_MAX_AP;

    wifi_scan_config_t scan_config = {
        .ssid = NULL,
        .bssid = NULL,
        .channel = 0,
        .show_hidden = true
    };

    ESP_LOGI(TAG, "Escaneando redes WiFi visibles...");

    esp_wifi_disconnect();
    vTaskDelay(pdMS_TO_TICKS(500));

    esp_err_t ret = esp_wifi_scan_start(&scan_config, true);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Error iniciando escaneo WiFi: %s", esp_err_to_name(ret));
        return;
    }

    ret = esp_wifi_scan_get_ap_records(&ap_count, ap_records);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Error leyendo escaneo WiFi: %s", esp_err_to_name(ret));
        return;
    }

    ESP_LOGI(TAG, "Redes encontradas: %d", ap_count);

    for (int i = 0; i < ap_count; i++)
    {
        ESP_LOGI(TAG,
                 "%02d SSID='%s' RSSI=%d canal=%d auth=%s",
                 i + 1,
                 (char *) ap_records[i].ssid,
                 ap_records[i].rssi,
                 ap_records[i].primary,
                 auth_mode_name(ap_records[i].authmode));
    }
}
