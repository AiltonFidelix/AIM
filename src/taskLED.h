#ifndef TASKLED_H
#define TASKLED_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "esp_log.h"

#include "task_common.h"

#define LED GPIO_NUM_2

/**
 * @brief Create and start LED task
 * @param void 
 */
void start_task_led(void);

#endif