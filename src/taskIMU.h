#ifndef TASKIMU_H
#define TASKIMU_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sdkconfig.h"
#include "esp_log.h"

#include "tasks_common.h"
#include <mpu6050.h>

/**
 * @brief Create and start IMU task
 * @param void 
 */
void start_task_imu(void);

/* Functions to get sensor IMU values */
float getPitch();
float getRoll();
float getYaw();
float getTemperature();

#endif