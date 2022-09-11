#include "taskHTTP.h"

// TAG used for task http logi
static const char TAG[] = "task_http";

// Firmware update status
static int g_fw_update_status = OTA_UPDATE_PENDING;

// HTTP server task handle
static httpd_handle_t http_server_handle = NULL;

// HTTP server monitor task handle
static TaskHandle_t task_http_server_monitor = NULL;

// Queue handle used to manipulate the main queue of events
static QueueHandle_t http_server_monitor_queue_handle;

/**
 * @brief ESP32 timer configuration passed to esp_timer_create.
 */
const esp_timer_create_args_t fw_update_reset_args = {
    .callback = &http_server_fw_update_reset_callback,
    .arg = NULL,
    .dispatch_method = ESP_TIMER_TASK,
    .name = "fw_update_reset"};
esp_timer_handle_t fw_update_reset;

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
// extern const uint8_t favicon_ico_start[] asm("_binary_favicon_ico_start");
// extern const uint8_t favicon_ico_end[] asm("_binary_favicon_ico_end");

/**
 * @brief Checks the g_fw_update_status and creates the fw_update_reset timer if g_fw_update_status is true.
 */
static void http_server_fw_update_reset_timer(void)
{
    if (g_fw_update_status == OTA_UPDATE_SUCCESSFUL)
    {
        ESP_LOGI(TAG, "FW updated successful starting FW update reset timer");

        // Give the web page a chance to receive an acknowledge back and initialize the timer
        ESP_ERROR_CHECK(esp_timer_create(&fw_update_reset_args, &fw_update_reset));
        ESP_ERROR_CHECK(esp_timer_start_once(fw_update_reset, 8000000));
    }
    else
    {
        ESP_LOGE(TAG, "FW update unsuccessful");
    }
}

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
            case HTTP_MSG_OTA_UPDATE_SUCCESSFUL:
                ESP_LOGI(TAG, "HTTP_MSG_OTA_UPDATE_SUCCESSFUL");
                g_fw_update_status = OTA_UPDATE_SUCCESSFUL;
                http_server_fw_update_reset_timer();
                break;

            case HTTP_MSG_OTA_UPDATE_FAILED:
                ESP_LOGI(TAG, "HTTP_MSG_OTA_UPDATE_FAILED");
                g_fw_update_status = OTA_UPDATE_FAILED;
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
// static esp_err_t http_server_favicon_ico_handler(httpd_req_t *req)
// {
//     ESP_LOGI(TAG, "favicon.ico requested");

//     httpd_resp_set_type(req, "image/x-icon");
//     httpd_resp_send(req, (const char *)favicon_ico_start, favicon_ico_end - favicon_ico_start);

//     return ESP_OK;
// }

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
 * @brief Receives the .bin file fia the web page and handles the firmware update
 * @param req HTTP request for which the uri needs to be handled.
 * @return ESP_OK, otherwise ESP_FAIL if timeout occurs and the update cannot be started.
 */
esp_err_t http_server_OTA_update_handler(httpd_req_t *req)
{
    esp_ota_handle_t update_handle;

    char ota_buff[1024];
    int content_length = req->content_len;
    int content_received = 0;
    int recv_len;
    bool is_req_body_started = false;
    bool flash_successful = false;

    const esp_partition_t *update_partition = esp_ota_get_next_update_partition(NULL);

    // ESP_LOGI(TAG, "ESP number of ota partitions %d", esp_ota_get_app_partition_count());

    do
    {
        // Read the data for the request
        if ((recv_len = httpd_req_recv(req, ota_buff, MIN(content_length, sizeof(ota_buff)))) < 0)
        {
            // Check if timeout occurred
            if (recv_len == HTTPD_SOCK_ERR_TIMEOUT)
            {
                ESP_LOGE(TAG, "Socket Timeout");
                continue; ///> Retry receiving if timeout occurred
            }
            ESP_LOGE(TAG, "OTA other Error %d", recv_len);
            return ESP_FAIL;
        }
        ESP_LOGI(TAG, "OTA RX: %d of %d", content_received, content_length);

        // Is this the first data we are receiving
        // If so, it will have the information in the header that we need.
        if (!is_req_body_started)
        {
            is_req_body_started = true;

            // Get the location of the .bin file content (remove the web form data)
            char *body_start_p = strstr(ota_buff, "\r\n\r\n") + 4;
            int body_part_len = recv_len - (body_start_p - ota_buff);

            ESP_LOGI(TAG, "OTA file size: %d", content_length);

            esp_err_t ret = esp_ota_begin(update_partition, OTA_SIZE_UNKNOWN, &update_handle);
            if (ret != ESP_OK)
            {
                ESP_LOGE(TAG, "Error %s with OTA begin, cancelling OTA", esp_err_to_name(ret));
                http_server_monitor_send_message(HTTP_MSG_OTA_UPDATE_FAILED);
                return ret;
            }
            else
            {
                ESP_LOGI(TAG, "Writing to partition subtype %d at offset 0x%x", update_partition->subtype, update_partition->address);
            }

            // Write this first part of the data
            esp_ota_write(update_handle, body_start_p, body_part_len);
            content_received += body_part_len;
        }
        else
        {
            // Write OTA data
            esp_ota_write(update_handle, ota_buff, recv_len);
            content_received += recv_len;
        }

    } while (recv_len > 0 && content_received < content_length);

    if (esp_ota_end(update_handle) == ESP_OK)
    {
        // Lets update the partition
        if (esp_ota_set_boot_partition(update_partition) == ESP_OK)
        {
            const esp_partition_t *boot_partition = esp_ota_get_boot_partition();
            ESP_LOGI(TAG, "Next boot partition subtype %d at offset 0x%x", boot_partition->subtype, boot_partition->address);
            flash_successful = true;
        }
        else
        {
            ESP_LOGE(TAG, "FLASHED ERROR!!!");
        }
    }
    else
    {
        ESP_LOGE(TAG, "esp_ota_end ERROR!!!");
    }

    // We won't update the global variables throughout the file, so send the message about the status
    if (flash_successful)
    {
        http_server_monitor_send_message(HTTP_MSG_OTA_UPDATE_SUCCESSFUL);
    }
    else
    {
        http_server_monitor_send_message(HTTP_MSG_OTA_UPDATE_FAILED);
    }

    return ESP_OK;
}

/**
 * OTA status handler responds with the firmware update status after the OTA update is started
 * and responds with the compile time/date when the page is first requested
 * @param req HTTP request for which the uri needs to be handled
 * @return ESP_OK
 */
esp_err_t http_server_OTA_status_handler(httpd_req_t *req)
{
    char otaJSON[100];

    ESP_LOGI(TAG, "OTAstatus requested");

    sprintf(otaJSON, "{\"ota_update_status\":%d,\"compile_time\":\"%s\",\"compile_date\":\"%s\"}", g_fw_update_status, __TIME__, __DATE__);

    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, otaJSON, strlen(otaJSON));

    return ESP_OK;
}

/**
 * @brief Set the setDateTime.json file when the web page require.
 * @param req HTTP request for which the uri needs to be handled.
 * @return ESP_OK
 */
static esp_err_t http_server_set_date_time_json_handler(httpd_req_t *req)
{
    ESP_LOGI(TAG, "setDateTime.json requested");

    size_t len_day = 0,
           len_month = 0,
           len_year = 0,
           len_hour = 0,
           len_minute = 0;

    char *day_str = NULL,
         *month_str = NULL,
         *year_str = NULL,
         *hour_str = NULL,
         *minute_str = NULL;

    // Get date header
    len_day = httpd_req_get_hdr_value_len(req, "day") + 1;
    len_month = httpd_req_get_hdr_value_len(req, "month") + 1;
    len_year = httpd_req_get_hdr_value_len(req, "year") + 1;

    if ((len_day > 1) && (len_month > 1) && (len_year > 1))
    {
        day_str = malloc(len_day);
        month_str = malloc(len_month);
        year_str = malloc(len_year);

        esp_err_t day_ret = httpd_req_get_hdr_value_str(req, "day", day_str, len_day);
        esp_err_t month_ret = httpd_req_get_hdr_value_str(req, "month", month_str, len_month);
        esp_err_t year_ret = httpd_req_get_hdr_value_str(req, "year", year_str, len_year);

        if ((day_ret == ESP_OK) && (month_ret == ESP_OK) && (year_ret == ESP_OK))
        {
            ds1307SetDate(atoi(day_str), atoi(month_str), atoi(year_str), 0);
        }
        free(day_str);
        free(month_str);
        free(year_str);
    }

    // Get time header
    len_hour = httpd_req_get_hdr_value_len(req, "hour") + 1;
    len_minute = httpd_req_get_hdr_value_len(req, "minute") + 1;

    if ((len_hour > 1) && (len_minute > 1))
    {
        hour_str = malloc(len_hour);
        minute_str = malloc(len_minute);

        esp_err_t hour_ret = httpd_req_get_hdr_value_str(req, "hour", hour_str, len_hour);
        esp_err_t minute_ret = httpd_req_get_hdr_value_str(req, "minute", minute_str, len_minute);

        if ((hour_ret == ESP_OK) && (minute_ret == ESP_OK))
        {
            ds1307SetTime(atoi(hour_str), atoi(minute_str), 0);
        }
        free(hour_str);
        free(minute_str);
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
        // httpd_uri_t favicon_ico = {
        //     .uri = "/favicon.ico",
        //     .method = HTTP_GET,
        //     .handler = http_server_favicon_ico_handler,
        //     .user_ctx = NULL};
        // httpd_register_uri_handler(http_server_handle, &favicon_ico);

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

        // register OTAupdate handler
        httpd_uri_t OTA_update = {
            .uri = "/OTAupdate",
            .method = HTTP_POST,
            .handler = http_server_OTA_update_handler,
            .user_ctx = NULL};
        httpd_register_uri_handler(http_server_handle, &OTA_update);

        // register OTAstatus handler
        httpd_uri_t OTA_status = {
            .uri = "/OTAstatus",
            .method = HTTP_POST,
            .handler = http_server_OTA_status_handler,
            .user_ctx = NULL};
        httpd_register_uri_handler(http_server_handle, &OTA_status);

        // register /setDateTime.json handler
        httpd_uri_t setDateTime_json = {
            .uri = "/setDateTime.json",
            .method = HTTP_POST,
            .handler = http_server_set_date_time_json_handler,
            .user_ctx = NULL};
        httpd_register_uri_handler(http_server_handle, &setDateTime_json);

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

void http_server_fw_update_reset_callback(void *arg)
{
    ESP_LOGI(TAG, "Timer timed-out, restarting the device");
    esp_restart();
}