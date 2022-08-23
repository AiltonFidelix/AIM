#include "esp_log.h"
#include "nvs_flash.h"

#include "taskWiFi.h"
#include "taskIMU.h"
#include "taskLED.h"
#include "sdStorage.h"

// TAG used for main logi
static const char TAG[] = "main";

void app_main()
{
    ESP_LOGI(TAG, "AIM device Starting...");

    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if(ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND){
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // Start all tasks
    start_task_wifi();
    start_task_led();
    start_task_imu();
    card_test();

    ESP_LOGI(TAG, "All tasks were started!");
}