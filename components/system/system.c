#include <stdbool.h>

#include "freertos/FreeRTOS.h"

#include "system.h"
#include "esp_log.h"
#include "i2c_drv.h"
#include "sensors.h"

/* FUNCTIONS DECLARATIONS */
void system_init();

/* Private variables */
static const char *TAG = "system";

static bool is_init = false;

void system_task(void *arg)
{
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

    // Initialize i2c
    i2c_drv_init();
    // Initialize the sensors
    sensors_init();

    is_init = true;
}