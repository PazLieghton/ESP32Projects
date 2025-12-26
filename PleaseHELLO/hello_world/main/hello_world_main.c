#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_system.h"

// Manual declaration if still missing
extern int hall_sensor_read(void);

static const char *TAG = "HALL_SENSOR";

void app_main(void)
{
    ESP_LOGI(TAG, "Hall Effect Sensor Demo Started");

    while (1) {
        int hall_value = hall_sensor_read();
        ESP_LOGI(TAG, "Hall Sensor Value: %d", hall_value);
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}