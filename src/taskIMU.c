#include "taskIMU.h"
#include "math.h"

// TAG used for task imu logi
static const char TAG[] = "task_imu";

// global IMU variables
float g_pitch, g_roll, g_yaw;

void vTaskIMU(void *pvParameters)
{
    (void)pvParameters;

    if (mpuBegin(MPU6050_ACCEL_RANGE_2G, MPU6050_GYRO_RANGE_250DPS, true) != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to begin MPU6050 device. "
                      "Deleting task.");
        vTaskDelete(NULL);
    }
    mpuSetFilterBandwidth(MPU6050_BAND_21_HZ);

    ds1307Begin(false);
    sdcard_config();

    uint16_t storageInterval = nvs_load_interval() > 0 ? nvs_load_interval() : 60;
    uint16_t seconds = 0;

    const float PI = 3.141592;

    while (1)
    {
        esp_err_t ret = mpuReadSensors();
        if (ret == ESP_OK)
        {
            float accX = mpuGetAccelerationX();
            float accY = mpuGetAccelerationY();
            float accZ = mpuGetAccelerationZ();

            g_pitch = (180 / PI) * atan(accX / sqrt((accY * accY) + (accZ * accZ)));
            g_roll = (180 / PI) * atan(accY / sqrt((accX * accX) + (accZ * accZ)));
            g_yaw = (180 / PI) * atan(sqrt((accY * accY) + (accX * accX)) / accZ);
         
            if (seconds >= storageInterval)
            {
                seconds = 0;
                char data[100];
                char timestamp[24];
                ds1307GetTimestamp(timestamp);
                sprintf(data, "%s,%.1f°,%.1f°,%.1f°,%.1f°C\n", timestamp, getPitch(), getRoll(), getYaw(), getTemperature());
                write_data(FILE_NAME, data);
            }
        }
        else
        {
            printf("Read sensors failed! Error: %s\n", esp_err_to_name(ret));
        }

        vTaskDelay(1000 / portTICK_PERIOD_MS);

        seconds++;
    }
}

float getPitch()
{
    return g_pitch;
}

float getRoll()
{
    return g_roll;
}

float getYaw()
{
    return g_yaw;
}

float getTemperature()
{
    return mpuGetTemperature();
}

void start_task_imu(void)
{
    ESP_LOGI(TAG, "Starting task IMU...");
    xTaskCreatePinnedToCore(&vTaskIMU, "vTaskIMU", TASK_IMU_SIZE, NULL, TASK_IMU_PRIORITY, NULL, TASK_IMU_CORE);
}
