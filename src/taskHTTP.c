#include "taskHTTP.h"
#include "taskIMU.h"
#include "nvsStorage.h"

// TAG used for task http logi
static const char TAG[] = "task_http";

// HTTP server task handle
static httpd_handle_t http_server_handle = NULL;

// HTTP server monitor task handle
static TaskHandle_t task_http_server_monitor = NULL;

// Queue handle used to manipulate the main queue of events
static QueueHandle_t http_server_monitor_queue_handle;

// Embedded files: bootstrap.min.css, jquery.min.js, index.min.html, app.min.js, app.min.css and favicon.ico files
extern const uint8_t bootstrap_min_css_start[] asm("_binary_bootstrap_min_css_start");
extern const uint8_t bootstrap_min_css_end[] asm("_binary_bootstrap_min_css_end");
extern const uint8_t jquery_min_js_start[] asm("_binary_jquery_min_js_start");
extern const uint8_t jquery_min_js_end[] asm("_binary_jquery_min_js_end");
extern const uint8_t index_min_html_start[] asm("_binary_index_min_html_start");
extern const uint8_t index_min_html_end[] asm("_binary_index_min_html_end");
extern const uint8_t app_min_js_start[] asm("_binary_app_min_js_start");
extern const uint8_t app_min_js_end[] asm("_binary_app_min_js_end");
extern const uint8_t app_min_css_start[] asm("_binary_app_min_css_start");
extern const uint8_t app_min_css_end[] asm("_binary_app_min_css_end");
extern const uint8_t favicon_ico_start[] asm("_binary_favicon_ico_start");
extern const uint8_t favicon_ico_end[] asm("_binary_favicon_ico_end");

/**
 * @brief HTTP server monitor task used to track events of the HTTP server
 * @param pvParameters parameter which can be passed to the task.
 */
static void vTaskHTTPMonitor(void *pvParameters)
{
    (void)pvParameters;

    http_server_queue_message_t msg;

    while (1)
    {
        if (xQueueReceive(http_server_monitor_queue_handle, &msg, portMAX_DELAY))
        {
            switch (msg.msgID)
            {
            case HTTP_MSG_WIFI_CONNECT_INIT:
                ESP_LOGI(TAG, "HTTP_MSG_WIFI_CONNECT_INIT");

                break;

            case HTTP_MSG_WIFI_CONNECT_SUCCESS:
                ESP_LOGI(TAG, "HTTP_MSG_WIFI_CONNECT_SUCCESS");

                break;

            case HTTP_MSG_WIFI_CONNECT_FAIL:
                ESP_LOGI(TAG, "HTTP_MSG_WIFI_CONNECT_FAIL");

                break;

            case HTTP_MSG_OTA_UPDATE_SUCCESSFUL:
                ESP_LOGI(TAG, "HTTP_MSG_OTA_UPDATE_SUCCESSFUL");

                break;

            case HTTP_MSG_OTA_UPDATE_FAILED:
                ESP_LOGI(TAG, "HTTP_MSG_OTA_UPDATE_FAILED");

                break;

            case HTTP_MSG_OTA_UPATE_INITIALIZED:
                ESP_LOGI(TAG, "HTTP_MSG_OTA_UPATE_INITIALIZED");

                break;

            default:
                break;
            }
        }
    }
}

/**
 * @brief bootstrap.min.css get handler is requested when accessing the web page.
 * @param req HTTP request for which the uri needs to be handled.
 * @return ESP_OK
 */
static esp_err_t http_server_bootstrap_min_css_handler(httpd_req_t *req)
{
    ESP_LOGI(TAG, "bootstrap.min.css requested");

    httpd_resp_set_type(req, "text/css");
    httpd_resp_send(req, (const char *)bootstrap_min_css_start, bootstrap_min_css_end - bootstrap_min_css_start);

    return ESP_OK;
}

/**
 * @brief app.min.css get handler is requested when accessing the web page.
 * @param req HTTP request for which the uri needs to be handled.
 * @return ESP_OK
 */
static esp_err_t http_server_app_min_css_handler(httpd_req_t *req)
{
    ESP_LOGI(TAG, "app.min.css requested");

    httpd_resp_set_type(req, "text/css");
    httpd_resp_send(req, (const char *)app_min_css_start, app_min_css_end - app_min_css_start);

    return ESP_OK;
}

/**
 * @brief jquery.min.js get handler is requested when accessing the web page.
 * @param req HTTP request for which the uri needs to be handled.
 * @return ESP_OK
 */
static esp_err_t http_server_jquery_min_js_handler(httpd_req_t *req)
{
    ESP_LOGI(TAG, "jquery.min.js requested");

    httpd_resp_set_type(req, "application/javascript");
    httpd_resp_send(req, (const char *)jquery_min_js_start, jquery_min_js_end - jquery_min_js_start);

    return ESP_OK;
}

/**
 * @brief Sends the index.min.html page.
 * @param req HTTP request for which the uri needs to be handled.
 * @return ESP_OK
 */
static esp_err_t http_server_index_min_html_handler(httpd_req_t *req)
{
    ESP_LOGI(TAG, "index.min.html requested");

    httpd_resp_set_type(req, "text/html");
    httpd_resp_send(req, (const char *)index_min_html_start, index_min_html_end - index_min_html_start);

    return ESP_OK;
}

/**
 * @brief app.min.js get handler is requested when accessing the web page.
 * @param req HTTP request for which the uri needs to be handled.
 * @return ESP_OK
 */
static esp_err_t http_server_app_min_js_handler(httpd_req_t *req)
{
    ESP_LOGI(TAG, "app.min.js requested");

    httpd_resp_set_type(req, "application/javascript");
    httpd_resp_send(req, (const char *)app_min_js_start, app_min_js_end - app_min_js_start);

    return ESP_OK;
}

/**
 * @brief Sends the .ico (icon) file when accessing the web page.
 * @param req HTTP request for which the uri needs to be handled.
 * @return ESP_OK
 */
static esp_err_t http_server_favicon_ico_handler(httpd_req_t *req)
{
    ESP_LOGI(TAG, "favicon.ico requested");

    httpd_resp_set_type(req, "image/x-icon");
    httpd_resp_send(req, (const char *)favicon_ico_start, favicon_ico_end - favicon_ico_start);

    return ESP_OK;
}

/**
 * @brief Sends the sensoValues.json file when the web page require.
 * @param req HTTP request for which the uri needs to be handled.
 * @return ESP_OK
 */
static esp_err_t http_server_sensorValues_json_handler(httpd_req_t *req)
{
    ESP_LOGI(TAG, "sensorValues.json requested");

    char sensor_json[100];

    float pitch = getPitch();
    float roll = getRoll();
    float yaw = getYaw();
    float temperature = getTemperature();

    sprintf(sensor_json, "{\"pitch\":\"%.1f°\",\"roll\":\"%.1f°\",\"yaw\":\"%.1f°\",\"temperature\":\"%.1f°C\"}", pitch, roll, yaw, temperature);

    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, sensor_json, strlen(sensor_json));

    return ESP_OK;
}

/**
 * @brief Get the setInterval.json file when the web page require.
 * @param req HTTP request for which the uri needs to be handled.
 * @return ESP_OK
 */
static esp_err_t http_server_get_interval_json_handler(httpd_req_t *req)
{
    ESP_LOGI(TAG, "getInterval.json requested");

    char interval_json[100];

    sprintf(interval_json, "{\"interval\":\"%d\"}", nvs_load_interval());

    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, interval_json, strlen(interval_json));

	return ESP_OK;
}

/**
 * @brief Set the setInterval.json file when the web page require.
 * @param req HTTP request for which the uri needs to be handled.
 * @return ESP_OK
 */
static esp_err_t http_server_set_interval_json_handler(httpd_req_t *req)
{
    ESP_LOGI(TAG, "setInterval.json requested");

    size_t len_interval = 0;
    char *interval_str = NULL;

	// Get interval header
	len_interval = httpd_req_get_hdr_value_len(req, "interval") + 1;
	if (len_interval > 1)
	{
        interval_str = malloc(len_interval);
		if (httpd_req_get_hdr_value_str(req, "interval", interval_str, len_interval) == ESP_OK)
		{
			ESP_LOGI(TAG, "New interval: %s", interval_str);
            nvs_save_interval(interval_str);
		}
        free(interval_str);
	}

	return ESP_OK;
}

/**
 * @brief Sets up the default httpd server configuration.
 * @return http server instance handle if successful, NULL otherwise.
 */
static httpd_handle_t http_server_configure(void)
{
    // Generate the default configuration
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    // Create HTTP server monitor task
    xTaskCreatePinnedToCore(&vTaskHTTPMonitor, "vTaskHTTPMonitor", TASK_HTTP_MONITOR_SIZE, NULL, TASK_HTTP_MONITOR_PRIORITY, &task_http_server_monitor, TASK_HTTP_MONITOR_CORE);

    // Create the message queue
    http_server_monitor_queue_handle = xQueueCreate(3, sizeof(http_server_queue_message_t));

    // The core that the HTTP server will run on
    config.core_id = TASK_HTTP_SERVER_CORE;

    // Adjust the default priority to 1 less than the wifi application task
    config.task_priority = TASK_HTTP_SERVER_PRIORITY;

    // Bump up the stack size (default is 4096)
    config.stack_size = TASK_HTTP_SERVER_SIZE;

    // Increase uri handlers
    config.max_uri_handlers = 20;

    // Increase the timeout limits
    config.recv_wait_timeout = 10;
    config.send_wait_timeout = 10;

    ESP_LOGI(TAG,
             "Starting server on port: '%d' with task priority: '%d'",
             config.server_port,
             config.task_priority);

    // Start the httpd server
    if (httpd_start(&http_server_handle, &config) == ESP_OK)
    {
        ESP_LOGI(TAG, "Registering URI handlers");

        // register bootstrap.min.css handler
        httpd_uri_t bootstrap_min_css = {
            .uri = "/bootstrap.min.css",
            .method = HTTP_GET,
            .handler = http_server_bootstrap_min_css_handler,
            .user_ctx = NULL};
        httpd_register_uri_handler(http_server_handle, &bootstrap_min_css);

        // register app.min.css handler
        httpd_uri_t app_min_css = {
            .uri = "/app.min.css",
            .method = HTTP_GET,
            .handler = http_server_app_min_css_handler,
            .user_ctx = NULL};
        httpd_register_uri_handler(http_server_handle, &app_min_css);

        // register jquery.min.js handler
        httpd_uri_t jquery_min_js = {
            .uri = "/jquery.min.js",
            .method = HTTP_GET,
            .handler = http_server_jquery_min_js_handler,
            .user_ctx = NULL};
        httpd_register_uri_handler(http_server_handle, &jquery_min_js);

        // register index.min.html handler
        httpd_uri_t index_min_html = {
            .uri = "/",
            .method = HTTP_GET,
            .handler = http_server_index_min_html_handler,
            .user_ctx = NULL};
        httpd_register_uri_handler(http_server_handle, &index_min_html);

        // register app.min.js handler
        httpd_uri_t app_min_js = {
            .uri = "/app.min.js",
            .method = HTTP_GET,
            .handler = http_server_app_min_js_handler,
            .user_ctx = NULL};
        httpd_register_uri_handler(http_server_handle, &app_min_js);

        // register favicon.ico handler
        httpd_uri_t favicon_ico = {
            .uri = "/favicon.ico",
            .method = HTTP_GET,
            .handler = http_server_favicon_ico_handler,
            .user_ctx = NULL};
        httpd_register_uri_handler(http_server_handle, &favicon_ico);

        // register /sensorValues.json handler
        httpd_uri_t sensorValues_json = {
            .uri = "/sensorValues.json",
            .method = HTTP_GET,
            .handler = http_server_sensorValues_json_handler,
            .user_ctx = NULL};
        httpd_register_uri_handler(http_server_handle, &sensorValues_json);

        // register /getInterval.json handler
        httpd_uri_t getInterval_json = {
            .uri = "/getInterval.json",
            .method = HTTP_GET,
            .handler = http_server_get_interval_json_handler,
            .user_ctx = NULL};
        httpd_register_uri_handler(http_server_handle, &getInterval_json);

        // register /setInterval.json handler
        httpd_uri_t setInterval_json = {
            .uri = "/setInterval.json",
            .method = HTTP_POST,
            .handler = http_server_set_interval_json_handler,
            .user_ctx = NULL};
        httpd_register_uri_handler(http_server_handle, &setInterval_json);

        return http_server_handle;
    }

    return NULL;
}

void start_task_http_server(void)
{
    ESP_LOGI(TAG, "Starting task HTTP...");

    if (http_server_handle == NULL)
    {
        http_server_handle = http_server_configure();
    }
}

void stop_task_http_server(void)
{
    ESP_LOGI(TAG, "Stoping task HTTP...");

    if (http_server_handle)
    {
        httpd_stop(http_server_handle);
        ESP_LOGI(TAG, "http_server_stop: stopping HTTP server");
        http_server_handle = NULL;
    }
    if (task_http_server_monitor)
    {
        vTaskDelete(task_http_server_monitor);
        ESP_LOGI(TAG, "http_server_stop: stopping HTTP server monitor");
        task_http_server_monitor = NULL;
    }
}

BaseType_t http_server_monitor_send_message(http_server_message_e msgID)
{
    http_server_queue_message_t msg;
    msg.msgID = msgID;
    return xQueueSend(http_server_monitor_queue_handle, &msg, portMAX_DELAY);
}