#include <stdbool.h>

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"

#include "system.h"
#include "esp_log.h"

/* Private variables */
static const char *TAG = "system";

static bool is_init = false;

SemaphoreHandle_t can_start_mutex;
static StaticSemaphore_t can_start_mutex_buffer;

void system_task(void *arg)
{
    // TODO: Turn a led on (status) and init esp-now (wifi connection)

    system_init();
}

// This must be the first module to be initialized!
void system_init()
{
    if (is_init)
    {
        return;
    }

    ESP_LOGI(TAG, "Initializing drone!!");

    can_start_mutex = xSemaphoreCreateMutexStatic(&can_start_mutex_buffer);
    xSemaphoreTake(can_start_mutex, portMAX_DELAY);

    // TODO: wifilink_init(), sysLoadInit()

    motors_init();

    is_init = true;
}