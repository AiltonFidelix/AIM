#include "nvsStorage.h"

// TAG used for nvs storage
static const char TAG[] = "nvs_storage";

static const char device[] = "AIM";

esp_err_t nvs_save_interval(const char *s_interval)
{
    esp_err_t ret;
    nvs_handle_t nvs_handle;

    ESP_LOGI(TAG, "Saving interval in the flash: %ss", s_interval);

    ret = nvs_open(device, NVS_READWRITE, &nvs_handle);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to open nvs");
        return ret;
    }

    uint16_t interval = atoi(s_interval);

    ret = nvs_set_i16(nvs_handle, "interval", interval);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Set interval failed");
        return ret;
    }

    ret = nvs_commit(nvs_handle);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Commit failed");
        return ret;
    }

    nvs_close(nvs_handle);

    ESP_LOGI(TAG, "Successful save interval, returned ESP_OK");

    return ESP_OK;
}

int16_t nvs_load_interval()
{
    esp_err_t ret;
    nvs_handle_t nvs_handle;

    int16_t interval = 0;

    ESP_LOGI(TAG, "Loading interval from flash");

    if (nvs_open(device, NVS_READONLY, &nvs_handle) == ESP_OK)
    {
        ret = nvs_get_i16(nvs_handle, "interval", &interval);
        if (ret != ESP_OK)
        {
            ESP_LOGE(TAG, "Failed to load interval");
            return 0;
        }
        nvs_close(nvs_handle);

        ESP_LOGI(TAG, "Interval load: %d", interval);

        return interval;
    }
    return 0;
}