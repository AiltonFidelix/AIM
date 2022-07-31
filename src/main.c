#include "esp_err.h"

#include "config.h"
#include <mpu6050.h>

// TAG used for main application logi
static const char TAG[] = "main";

void vTaskLED(void *pvParameters)
{
    (void)pvParameters;

    bool led_state = false;
    gpio_set_direction(LED, GPIO_MODE_OUTPUT);

    while (1)
    {
        // Toggle LED
        gpio_set_level(LED, led_state);
        led_state = !led_state;
        vTaskDelay(80 / portTICK_PERIOD_MS);
    }
}

void vTaskIMU(void *pvParameters)
{
    (void)pvParameters;

    mpuBegin();
    mpuSetFilterBandwidth(MPU6050_BAND_5_HZ);

    while (1)
    {
        esp_err_t ret = mpuReadSensors();
        if (ret == ESP_OK)
        {
            printf("Temperature: %.2f°C\n", mpuReadTemperature());
            printf("Acceleration X: %.2f\n", mpuReadAccelerationX());
            printf("Acceleration Y: %.2f\n", mpuReadAccelerationY());
            printf("Acceleration Z: %.2f\n", mpuReadAccelerationZ());
            printf("Gyroscope X: %.2f\n", mpuReadGyroscopeX());
            printf("Gyroscope Y: %.2f\n", mpuReadGyroscopeY());
            printf("Gyroscope Z: %.2f\n", mpuReadGyroscopeZ());
        }
        else{
            printf("Read sensors failed! Error: %s\n", esp_err_to_name(ret));
        }
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

void app_main()
{
    ESP_LOGI(TAG, "AIM device Starting...");

    xTaskCreate(&vTaskLED, "vTaskLED", 2048, NULL, 5, NULL);
    xTaskCreate(&vTaskIMU, "vTaskIMU", 2048, NULL, 5, NULL);
}