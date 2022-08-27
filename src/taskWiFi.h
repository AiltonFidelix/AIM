#ifndef TASKWIFI_H
#define TASKWIFI_H

#include "tasks_common.h"
#include "esp_netif.h"

#define WIFI_AP_SSID "AIM-device"        // AP name
#define WIFI_AP_PASSWORD "12345678"      // AP password
#define WIFI_AP_CHANNEL 1                // AP channel
#define WIFI_AP_SSID_HIDDEN 0            // AP visibility
#define WIFI_AP_MAX_CONNECTIONS 1        // AP max clientes
#define WIFI_AP_BEACON_INTERVAL 100      // AP beacon: 100 milliseconds recommended
#define WIFI_AP_IP "192.168.4.1"         // AP default IP address
#define WIFI_AP_GATEWAY "191.168.4.1"    // AP default gateway (should be the same as the IP)
#define WIFI_AP_NETMASK "255.255.255.0"  // AP default netmask
#define WIFI_AP_BANDWIDTH WIFI_BW_HT20   // AP bandwidth with 20MHz (40MHz is the other option)
#define WIFI_STA_POWER_SAVE WIFI_PS_NONE // Power save not used
#define MAX_SSID_LENGHT 32               // IEEE standard maximum
#define MAX_PASSWORD_LENGHT 64           // IEEE standard maximum
#define MAX_CONNECTION_RETRIES 5         // Retry number on disconnected

// netif object for Station and Access Point
extern esp_netif_t *esp_netif_sta;
extern esp_netif_t *esp_netif_ap;

/**
 * @brief Message IDs for the WIFI application task
 */
typedef enum wifi_app_message
{
    WIFI_APP_MSG_START_HTTP_SERVER,
    WIFI_APP_MSG_CONNECTING_FROM_HTTP_SERVER,
    WIFI_APP_MSG_STA_CONNECTED_GOT_IP,
    WIFI_APP_MSG_STA_DISCONNECTED,
} wifi_app_message_e;

/**
 * @brief Struct for the message queue
 */
typedef struct wifi_app_queue_message
{
    wifi_app_message_e msgID;
} wifi_app_queue_message_t;

/**
 * @brief Send a message to the queue
 * @param msgID message ID from the wifi_app_message_e enum
 * @return pdTRUE if a item was succesfully sent to the queue, otherwise pdFALSE
 */
BaseType_t wifi_app_send_message(wifi_app_message_e msgID);

/**
 * @brief Create and start WIFI task
 * @param void
 */
void start_task_wifi(void);

/**
 * @brief Gets the wifi configuration
 * @param void
 */
wifi_config_t *wifi_app_get_wifi_config(void);

#endif