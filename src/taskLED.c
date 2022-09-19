#include "taskLED.h"

// TAG used for task led logi
static const char TAG[] = "task_led";

void vTaskLED(void *pvParameters)
{
    (void)pvParameters;

    uint16_t time_blink = 1000;
    uint8_t time_bounce = 5;
    uint8_t bounce = 0;
    gpio_set_direction(LED, GPIO_MODE_OUTPUT);
    gpio_set_direction(CFG_BUTTON, GPIO_MODE_INPUT);

    bool led_state = false;

    eTaskState taskWiFiState;

    while (1)
    {
        // Toggle LED
        gpio_set_level(LED, led_state);
        led_state = !led_state;

        if (!gpio_get_level(CFG_BUTTON))
        {
            bounce++;
            if (bounce == time_bounce)
            {
                bounce = 0;
                taskWiFiState = status_task_wifi();

                if (taskWiFiState == eSuspended || taskWiFiState == eInvalid)
                {
                    time_blink = 80;
                    start_task_wifi();
                }
                else
                {
                    time_blink = 1000;
                    stop_task_wifi();
                }
            }
        }
        else
        {
            bounce = 0;
        }
        
        time_bounce = time_blink == 1000 ? 5 : 62;

        vTaskDelay(time_blink / portTICK_PERIOD_MS);
    }
}

void start_task_led(void)
{
    ESP_LOGI(TAG, "Starting task LED...");
    xTaskCreatePinnedToCore(&vTaskLED, "vTaskLED", TASK_LED_SIZE, NULL, TASK_LED_PRIORITY, NULL, TASK_LED_CORE);
}