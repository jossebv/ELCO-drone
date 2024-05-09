#include <stdbool.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"

#include "main.h"
#include "system.h"
#include "comms.h"
#include "led.h"
#include "esp_log.h"
#include "i2c_drv.h"
#include "sensors.h"
#include "motors.h"
#include "wifi.h"
#include "adc.h"
#include "nvs_flash.h"
#include "controller.h"

/* FUNCTIONS DECLARATIONS */
void system_init();

/* Private variables */
static const char *TAG = "system";

static bool is_init = false;
static fsm_t *drone_fsm;

/**
 * @brief Task for the system
 *
 * @param arg pointer to the arguments, not used
 */
void system_task(void *arg)
{
    /* Initialize the system */
    system_init();

    /* Crate timer variables */
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = pdMS_TO_TICKS(DRONE_UPDATE_MS);
    BaseType_t xWasDelayed;

    /* Create the fsms */
    fsm_t *green_led_fsm = led_fsm_create(GREEN_LED_PIN);
    fsm_t *blue_led_fsm = led_fsm_create(BLUE_LED_PIN);
    fsm_t *red_led_fsm = led_fsm_create(RED_LED_PIN);

    drone_fsm = system_fsm_create(green_led_fsm, blue_led_fsm, red_led_fsm);

    while (1)
    {
        xWasDelayed = xTaskDelayUntil(&xLastWakeTime, xFrequency);

        if (xWasDelayed == pdFALSE)
        {
            // ESP_LOGW(TAG, "Task was delayed");
        }

        fsm_fire(drone_fsm);
        fsm_fire(green_led_fsm);
        fsm_fire(blue_led_fsm);
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
    comms_init();
    // Initialize i2c
    vTaskDelay(pdMS_TO_TICKS(100));
    i2c_drv_init();
    // Initialize the motors
    vTaskDelay(pdMS_TO_TICKS(100));
    motors_init();
    // Initialize the sensors
    vTaskDelay(pdMS_TO_TICKS(100));
    sensors_init();

    // Initialize the adc
    adc_init();

    vTaskDelay(pdMS_TO_TICKS(1000));

    is_init = true;
}