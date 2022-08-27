#include "taskWiFi.h"
#include "taskHTTP.h"

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"

#include "esp_err.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "lwip/netdb.h"

// TAG used for task wifi logi
static const char TAG[] = "task_wifi";

// Used to returning the wifi configuration
wifi_config_t *wifi_config = NULL;

// Used to track the number for retries when a connection attempt fails
static int g_retry_number;

// Queue handle used to manipulate the main queue of events
static QueueHandle_t wifi_app_queue_handle;

// netif object for Station and Access Point
esp_netif_t *esp_netif_sta = NULL;
esp_netif_t *esp_netif_ap = NULL;

/**
 * @brief  WiFi application event handler
 * @param arg_data aside from event data, that is passed to the handler when it is called
 * @param event_base the base id of the event to register the handler for
 * @param event_id the id of the event to register the handler for
 * @param event_data event data
 */
static void wifi_app_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT)
    {
        switch (event_id)
        {
        case WIFI_EVENT_AP_START:
            ESP_LOGI(TAG, "WIFI_EVENT_AP_START");
            break;
        case WIFI_EVENT_AP_STOP:
            ESP_LOGI(TAG, "WIFI_EVENT_AP_STOP");
            break;
        case WIFI_EVENT_AP_STACONNECTED:
            ESP_LOGI(TAG, "WIFI_EVENT_AP_STACONNECTED");
            break;
        case WIFI_EVENT_AP_STADISCONNECTED:
            ESP_LOGI(TAG, "WIFI_EVENT_AP_STADISCONNECTED");
            break;
        case WIFI_EVENT_STA_START:
            ESP_LOGI(TAG, "WIFI_EVENT_STA_START");
            break;
        case WIFI_EVENT_STA_CONNECTED:
            ESP_LOGI(TAG, "WIFI_EVENT_STA_CONNECTED");
            break;
        case WIFI_EVENT_STA_DISCONNECTED:
            ESP_LOGI(TAG, "WIFI_EVENT_STA_DISCONNECTED");
            wifi_event_sta_disconnected_t *wifi_event_sta_disconnected = (wifi_event_sta_disconnected_t *)malloc(sizeof(wifi_event_sta_disconnected_t));
            *wifi_event_sta_disconnected = *((wifi_event_sta_disconnected_t *)event_data);
            printf("WIFI_EVENT_STA_DISCONNECTED, reason code %d\n", wifi_event_sta_disconnected->reason);

            if (g_retry_number < MAX_CONNECTION_RETRIES)
            {
                esp_wifi_connect();
                g_retry_number++;
            }
            else
            {
                wifi_app_send_message(WIFI_APP_MSG_STA_DISCONNECTED);
            }
            break;
        default:
            break;
        }
    }
    else if (event_base == IP_EVENT)
    {
        switch (event_id)
        {
        case IP_EVENT_STA_GOT_IP:
            ESP_LOGI(TAG, "IP_EVENT_STA_GOT_IP");
            wifi_app_send_message(WIFI_APP_MSG_STA_CONNECTED_GOT_IP);
            break;
        default:
            break;
        }
    }
}

/**
 * @brief Initializes the WiFi application event handler for wifi and IP events.
 */
static void wifi_app_event_handler_init(void)
{
    // Event loop for the wifi driver
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    // Event handler for the connectino
    esp_event_handler_instance_t instance_wifi_event;
    esp_event_handler_instance_t instance_ip_event;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_app_event_handler, NULL, &instance_wifi_event));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, ESP_EVENT_ANY_ID, &wifi_app_event_handler, NULL, &instance_ip_event));
}

/**
 * @brief Initializes the TCP stack and default WiFi configuration.
 */
static void wifi_app_default_wifi_init(void)
{
    // Initialize the TCP stack
    ESP_ERROR_CHECK(esp_netif_init());

    // Default Wifi config - operations must be in this order!
    wifi_init_config_t wifi_init_config = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&wifi_init_config));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
    esp_netif_sta = esp_netif_create_default_wifi_sta();
    esp_netif_ap = esp_netif_create_default_wifi_ap();
}

static void wifi_app_soft_ap_config(void)
{
    // SoftAP - WiFi access point configuration
    wifi_config_t ap_config = {
        .ap = {
            .ssid = WIFI_AP_SSID,
            .ssid_len = strlen(WIFI_AP_SSID),
            .password = WIFI_AP_PASSWORD,
            .channel = WIFI_AP_CHANNEL,
            .ssid_hidden = WIFI_AP_SSID_HIDDEN,
            .authmode = WIFI_AUTH_WPA2_PSK,
            .max_connection = WIFI_AP_MAX_CONNECTIONS,
            .beacon_interval = WIFI_AP_BEACON_INTERVAL,
        },
    };

    // Configure DHCP for the AP
    esp_netif_ip_info_t ap_ip_info;
    memset(&ap_ip_info, 0x00, sizeof(ap_ip_info));

    esp_netif_dhcps_stop(esp_netif_ap); // must call this first
    inet_pton(AF_INET, WIFI_AP_IP, &ap_ip_info.ip);
    inet_pton(AF_INET, WIFI_AP_GATEWAY, &ap_ip_info.gw);
    inet_pton(AF_INET, WIFI_AP_NETMASK, &ap_ip_info.netmask);
    ESP_ERROR_CHECK(esp_netif_set_ip_info(esp_netif_ap, &ap_ip_info));
    ESP_ERROR_CHECK(esp_netif_dhcps_start(esp_netif_ap));

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_APSTA));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_AP, &ap_config));
    ESP_ERROR_CHECK(esp_wifi_set_bandwidth(WIFI_IF_AP, WIFI_AP_BANDWIDTH));
    ESP_ERROR_CHECK(esp_wifi_set_ps(WIFI_STA_POWER_SAVE));
}

/**
 * @brief Connects the ESP32 to an external AP using
 * the updated station configuration
 */
static void wifi_app_connect_sta(void)
{
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, wifi_app_get_wifi_config()));
    ESP_ERROR_CHECK(esp_wifi_connect());
}

/**
 * @brief Main task for the WiFi application
 * @param pvParameters
 */
static void vTaskWiFi(void *pvParameters)
{
    (void)pvParameters;

    wifi_app_queue_message_t msg;

    // initialize the event handle
    wifi_app_event_handler_init();

    // initialize the TCP/IP stack and wifi config
    wifi_app_default_wifi_init();

    // SoftAp config
    wifi_app_soft_ap_config();

    // Satrt Wifi
    ESP_ERROR_CHECK(esp_wifi_start());

    // Send first event message
    wifi_app_send_message(WIFI_APP_MSG_START_HTTP_SERVER);

    while (1)
    {
        if (xQueueReceive(wifi_app_queue_handle, &msg, portMAX_DELAY))
        {
            switch (msg.msgID)
            {
            case WIFI_APP_MSG_START_HTTP_SERVER:
                ESP_LOGI(TAG, "WIFI_APP_MSG_START_HTTP_SERVER");

                start_task_http_server();
                
                break;
            case WIFI_APP_MSG_CONNECTING_FROM_HTTP_SERVER:
                ESP_LOGI(TAG, "WIFI_APP_MSG_CONNECTING_FROM_HTTP_SERVER");

                // Attempt a connection
                wifi_app_connect_sta();

                // Set current number of retries to zero
                g_retry_number = 0;

                // Let the HTTP server know about the connection attempt
				http_server_monitor_send_message(HTTP_MSG_WIFI_CONNECT_INIT);

                break;
            case WIFI_APP_MSG_STA_CONNECTED_GOT_IP:
                ESP_LOGI(TAG, "WIFI_APP_MSG_STA_CONNECTED_GOT_IP");
                break;
            case WIFI_APP_MSG_STA_DISCONNECTED:
                ESP_LOGI(TAG, "WIFI_APP_MSG_STA_DISCONNECTED");
                break;
            default:
                break;
            }
        }
    }
}

BaseType_t wifi_app_send_message(wifi_app_message_e msgID)
{
    wifi_app_queue_message_t msg;
    msg.msgID = msgID;
    return xQueueSend(wifi_app_queue_handle, &msg, portMAX_DELAY);
}

wifi_config_t *wifi_app_get_wifi_config(void)
{
    return wifi_config;
}

void start_task_wifi(void)
{
    ESP_LOGI(TAG, "Starting task WiFi...");

    // disable wifi default logging messages
    esp_log_level_set("wifi", ESP_LOG_NONE);

    // Allocate memory for the wifi configuration
    wifi_config = (wifi_config_t *)malloc(sizeof(wifi_config_t));
    memset(wifi_config, 0x00, sizeof(wifi_config_t));

    wifi_app_queue_handle = xQueueCreate(3, sizeof(wifi_app_queue_message_t));

    // start the wifi application task
    xTaskCreatePinnedToCore(&vTaskWiFi, "vTaskWiFi", TASK_WIFI_SIZE, NULL, TASK_WIFI_PRIORITY, NULL, TASK_WIFI_CORE);
}