#include <stdbool.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"

#include "main.h"
#include "system.h"
#include "esp_log.h"
#include "i2c_drv.h"
#include "sensors.h"
#include "wifi.h"
#include "nvs_flash.h"
#include "controller.h"

/* FUNCTIONS DECLARATIONS */
void system_init();

/* Private variables */
static const char *TAG = "system";

static bool is_init = false;
static fsm_t *drone_fsm;

void system_task(void *arg)
{
    system_init();
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = pdMS_TO_TICKS(100);
    command_t command;
    drone_fsm = system_fsm_create();

    while (1)
    {
        // TODO: Here we will call fsm_fire() to update the state of the drone and execute the corresponding action
        // vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(DRONE_UPDATE_MS));
        // sensors_update_drone_data();

        xTaskDelayUntil(&xLastWakeTime, xFrequency);
        fsm_fire(drone_fsm);
        printf("FSM state: %d\n", drone_fsm->current_state);

        // get_command(&command);
        // get_gyroscope_data();
        // get_accelerometer_data();
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

    // Initialize nvs
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // Initialize wifi
    // wifi_init();
    // Initialize i2c
    i2c_drv_init();
    // Initialize the sensors
    sensors_init();

    // TODO: Initialize the FSM
    // drone_fsm = system_fsm_create();

    is_init = true;
}