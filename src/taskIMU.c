#include "taskIMU.h"
#include "math.h"

// TAG used for task imu logi
static const char TAG[] = "task_imu";

float square(float n)
{
    return n * n;
}

void vTaskIMU(void *pvParameters)
{
    (void)pvParameters;

    const float PI = 3.141592;
    int16_t i = 1;
    float grx, gry, grz;

    if (mpuBegin(MPU6050_ACCEL_RANGE_2G, MPU6050_GYRO_RANGE_250DPS) != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to begin MPU6050 device. "
                      "Deleting task.");
        vTaskDelete(NULL);
    }
    mpuSetFilterBandwidth(MPU6050_BAND_21_HZ);

    while (1)
    {
        esp_err_t ret = mpuReadSensors();
        if (ret == ESP_OK)
        {
            float accX = mpuGetAccelerationX();
            float accY = mpuGetAccelerationY();
            float accZ = mpuGetAccelerationZ();
            float gyroX = mpuGetGyroscopeX();
            float gyroY = mpuGetGyroscopeY();
            float gyroZ = mpuGetGyroscopeZ();
            float temp = mpuGetTemperature();

            // calculate accelerometer angles
            float arx = (180 / PI) * atan(accX / sqrt(square(accY) + square(accZ)));
            float ary = (180 / PI) * atan(accY / sqrt(square(accX) + square(accZ)));
            float arz = (180 / PI) * atan(sqrt(square(accY) + square(accX)) / accZ);

            // set initial values equal to accel values
            if (i == 1)
            {
                grx = arx;
                gry = ary;
                grz = arz;
            }
            // integrate to find the gyro angle
            else
            {
                grx = grx + (1 * gyroX);
                gry = gry + (1 * gyroY);
                grz = grz + (1 * gyroZ);
            }

            // apply filter
            float rx = (0.1 * arx) + (0.9 * grx);
            float ry = (0.1 * ary) + (0.9 * gry);
            float rz = (0.1 * arz) + (0.9 * grz);

            printf("Pitch: %.2f\n", rx);
            printf("Roll: %.2f\n", ry);
            printf("Yaw: %.2f\n", rz);

            i++;
        }
        else
        {
            printf("Read sensors failed! Error: %s\n", esp_err_to_name(ret));
        }
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

void start_task_imu(void)
{
    ESP_LOGI(TAG, "Starting task IMU...");
    xTaskCreatePinnedToCore(&vTaskIMU, "vTaskIMU", TASK_IMU_SIZE, NULL, TASK_IMU_PRIORITY, NULL, TASK_IMU_CORE);
}