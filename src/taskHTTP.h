#ifndef TASKHTTP_H
#define TASKHTTP_H

#include "esp_http_server.h"
#include "esp_log.h"
#include "esp_ota_ops.h"
#include "esp_wifi.h"
#include "sys/param.h"

#include "tasks_common.h"
#include "taskWiFi.h"
#include "taskIMU.h"
#include "nvsStorage.h"

#define OTA_UPDATE_PENDING 0
#define OTA_UPDATE_SUCCESSFUL 1
#define OTA_UPDATE_FAILED -1

/**
 * Messages for the HTTP monitor
 */
typedef enum http_server_message
{
	HTTP_MSG_WIFI_CONNECT_INIT = 0,
	HTTP_MSG_WIFI_CONNECT_SUCCESS,
	HTTP_MSG_WIFI_CONNECT_FAIL,
	HTTP_MSG_WIFI_USER_DISCONNECT,
	HTTP_MSG_OTA_UPDATE_SUCCESSFUL,
	HTTP_MSG_OTA_UPDATE_FAILED,
	HTTP_MSG_TIME_SERVICE_INITIALIZED,
} http_server_message_e;

/**
 * Structure for the message queue
 */
typedef struct http_server_queue_message
{
	http_server_message_e msgID;
} http_server_queue_message_t;

/**
 * Sends a message to the queue
 * @param msgID message ID from the http_server_message_e enum.
 * @return pdTRUE if an item was successfully sent to the queue, otherwise pdFALSE.
 * @note Expand the parameter list based on your requirements e.g. how you've expanded the http_server_queue_message_t.
 */
BaseType_t http_server_monitor_send_message(http_server_message_e msgID);

/**
 * @brief Start HTTP server task
 */
void start_task_http_server(void);

/**
 * Stops the HTTP server.
 */
void stop_task_http_server(void);

/**
 * @brief Timer callback function which calls esp_restart upon successful firmware update.
 */
void http_server_fw_update_reset_callback(void *arg);

#endif