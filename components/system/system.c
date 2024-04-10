#include <stdbool.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"

#include "main.h"
#include "system.h"
#include "led.h"
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
    /* Initialize the system */
    system_init();

    /* Crate timer variables */
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = pdMS_TO_TICKS(DRONE_UPDATE_MS);

    /* Create the fsms */
    fsm_t *led_fsm = led_fsm_create();
    drone_fsm = system_fsm_create();

    while (1)
    {
        // TODO: Here we will call fsm_fire() to update the state of the drone and execute the corresponding action
        // vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(DRONE_UPDATE_MS));
        // sensors_update_drone_data();

        xTaskDelayUntil(&xLastWakeTime, xFrequency);
        fsm_fire(drone_fsm);
        fsm_fire(led_fsm);
        // printf("FSM state: %d\n", drone_fsm->current_state);

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
    wifi_init();
    // Initialize the leds
    led_init();
    // Initialize i2c
    i2c_drv_init();
    // Initialize the sensors
    sensors_init();

    is_init = true;
}