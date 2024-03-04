#include <stdbool.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "system.h"
#include "esp_log.h"
#include "i2c_drv.h"
#include "sensors.h"

/* FUNCTIONS DECLARATIONS */
void system_init();

/* Private variables */
static const char *TAG = "system";

static bool is_init = false;
static fsm_t *drone_fsm;

void system_task(void *arg)
{
    system_init();

    while (1)
    {
        sensors_update_drone_data();
        // TODO: Here we will call fsm_fire() to update the state of the drone and execute the corresponding action
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
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

    // TODO: Initialize the FSM
    drone_fsm = system_fsm_create();

    is_init = true;
}