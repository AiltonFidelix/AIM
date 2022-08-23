#include "taskLED.h"

// TAG used for task led logi
static const char TAG[] = "task_led";

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

void start_task_led(void){
    ESP_LOGI(TAG, "Starting task LED...");
    xTaskCreatePinnedToCore(&vTaskLED, "vTaskLED", TASK_LED_SIZE, NULL, TASK_LED_PRIORITY, NULL, TASK_LED_CORE);
}