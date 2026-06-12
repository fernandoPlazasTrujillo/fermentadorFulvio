/**
 * @file wifi_manager.c
 * @brief Implementación del gestor de conectividad WiFi.
 *
 * Este módulo implementa la inicialización, conexión y monitoreo
 * de la interfaz WiFi del ESP32 utilizando el framework ESP-IDF.
 *
 * Funcionalidades principales:
 * - Inicialización del subsistema WiFi.
 * - Gestión automática de conexión y reconexión.
 * - Obtención de dirección IP mediante DHCP.
 * - Escaneo de redes inalámbricas disponibles.
 * - Consulta del estado de conexión.
 * - Sincronización mediante Event Groups de FreeRTOS.
 *
 * El módulo abstrae la complejidad de la pila de red y proporciona
 * una interfaz sencilla para las tareas de aplicación.
 *
 * Mecanismos FreeRTOS utilizados:
 * - Event Groups para sincronización de estados de conexión.
 *
 * Eventos gestionados:
 * - Inicio del modo estación.
 * - Conexión y desconexión WiFi.
 * - Obtención de dirección IP.
 * - Reintentos automáticos de conexión.
 *
 * @author
 * Fernando Plazas Trujillo
 * Isabella Ordoñez
 * Juan Daniel Constain
 */
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

#define WIFI_SSID      "Sala331"
#define WIFI_PASS      "Sala331tm"
//#define WIFI_SSID      "TP-LINK_DDAE"
//#define WIFI_PASS      "24890717"
//#define WIFI_SSID      "S25 Ultra de fernando"
//#define WIFI_PASS      "fer12345"

/**
 * @brief Número máximo de reintentos de conexión WiFi.
 */
#define WIFI_MAX_RETRIES 4

/**
 * @brief Número máximo de puntos de acceso almacenados durante un escaneo.
 */
#define WIFI_SCAN_MAX_AP 20

/**
 * @brief Etiqueta utilizada para mensajes de depuración.
 */
static const char *TAG = "wifi_manager";

/**
 * @brief Contador de reintentos de conexión WiFi.
 */
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
 * @brief Manejador de eventos WiFi e IP.
 *
 * Esta función es registrada en el sistema de eventos del ESP-IDF
 * y es ejecutada automáticamente cuando ocurre un evento relacionado
 * con la conectividad de red.
 *
 * Eventos gestionados:
 * - Inicio de la interfaz WiFi.
 * - Desconexión de la red.
 * - Obtención de dirección IP.
 *
 * Además actualiza los Event Groups utilizados por las tareas
 * para sincronizar el estado de conexión.
 *
 * @param arg Argumento de usuario.
 * @param event_base Base del evento recibido.
 * @param event_id Identificador del evento.
 * @param event_data Datos asociados al evento.
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
 * @brief Inicializa y conecta el ESP32 a una red WiFi.
 *
 * Realiza la inicialización de:
 * - NVS Flash.
 * - Pila TCP/IP.
 * - Event Loop del sistema.
 * - Interfaz WiFi en modo estación.
 *
 * Posteriormente configura las credenciales de acceso,
 * registra los manejadores de eventos y comienza el proceso
 * de conexión a la red inalámbrica.
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
/**
 * @brief Espera hasta que el ESP32 se conecte a la red WiFi.
 *
 * La función bloquea la ejecución hasta recibir un evento de
 * conexión exitosa o hasta que se detecte un fallo de conexión.
 *
 * @param timeout_ms Tiempo máximo de espera en milisegundos.
 *
 * @return true si la conexión fue exitosa.
 * @return false si ocurrió timeout o fallo de conexión.
 */
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

/**
 * @brief Convierte un modo de autenticación WiFi a texto.
 *
 * Se utiliza para mostrar información legible durante
 * el escaneo de redes disponibles.
 *
 * @param authmode Modo de autenticación reportado por ESP-IDF.
 *
 * @return Cadena descriptiva del tipo de autenticación.
 */
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

/**
 * @brief Realiza un escaneo de redes WiFi disponibles.
 *
 * Ejecuta un escaneo activo del entorno inalámbrico,
 * obtiene la lista de puntos de acceso detectados y
 * muestra por consola información relevante:
 *
 * - SSID
 * - Intensidad de señal (RSSI)
 * - Canal
 * - Tipo de autenticación
 *
 * Los resultados son utilizados principalmente para
 * diagnóstico y configuración del sistema.
 */
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

/**
 * @brief Verifica si existe una conexión WiFi activa.
 *
 * Consulta el estado actual almacenado en el Event Group
 * del módulo WiFi.
 *
 * @return true si el ESP32 está conectado a una red.
 * @return false si no existe conexión activa.
 */
bool wifi_is_connected(void)
{
    if (wifi_event_group == NULL)
    {
        return false;
    }

    EventBits_t bits = xEventGroupGetBits(wifi_event_group);

    return (bits & WIFI_CONNECTED_BIT) != 0;
}
