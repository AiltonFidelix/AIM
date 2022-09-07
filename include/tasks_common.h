#ifndef TASKSCOMMON_H
#define TASKSCOMMON_H

// Config task WIFI
#define TASK_WIFI_SIZE 4096
#define TASK_WIFI_PRIORITY 5
#define TASK_WIFI_CORE 0

// Config task HTTP server
#define TASK_HTTP_SERVER_SIZE 8192
#define TASK_HTTP_SERVER_PRIORITY 4
#define TASK_HTTP_SERVER_CORE 0

// Config task HTTP monitor
#define TASK_HTTP_MONITOR_SIZE 4096
#define TASK_HTTP_MONITOR_PRIORITY 3
#define TASK_HTTP_MONITOR_CORE 0

// Config task IMU
#define TASK_IMU_SIZE 4096
#define TASK_IMU_PRIORITY 5
#define TASK_IMU_CORE 1

// Config task LED
#define TASK_LED_SIZE 512
#define TASK_LED_PRIORITY 1
#define TASK_LED_CORE 1

#endif