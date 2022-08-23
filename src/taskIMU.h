#ifndef TASKIMU_H
#define TASKIMU_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sdkconfig.h"
#include "esp_log.h"

#include "task_common.h"
#include <mpu6050.h>

/**
 * @brief Create and start IMU task
 * @param void 
 */
void start_task_imu(void);

#endif