/**
 * @file ultrasonic.c
 * @author José Manuel Bravo
 * @brief File for controlling the ultrasonic sensor.
 * @version 0.1
 * @date 2024-04-03
 *
 * @copyright Copyright (c) 2024
 *
 */

/* INCLUDES */
#include <stdbool.h>
#include <rom/ets_sys.h>

#include "ultrasonic.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_timer.h"
#include "driver/gpio.h"

/* DEFINES */
#define ULTRASONIC_TRIGGER_PIN GPIO_NUM_13                         /**< Pin for the trigger of the ultrasonic sensor */
#define ULTRASONIC_ECHO_PIN GPIO_NUM_12                            /**< Pin for the echo of the ultrasonic sensor */
#define ULTRASONIC_TIMEOUT_US 10000                                /**< Timeout for the ultrasonic sensor */
#define ULTRASONIC_UPDATE_PERIOD_US (ULTRASONIC_TIMEOUT_US + 1000) /**< Period for updating the ultrasonic sensor */

/* VARIABLES */
static bool is_init = false;
static float distance = 0;
static TaskHandle_t ultrasonic_task_handle;

/* FUNCTIONS DECLARATIONS */

/* PUBLIC FUNCTIONS */

/**
 * @brief Measures the distance with the ultrasonic sensor
 *
 */
void ultrasonic_measure_distance()
{

    // Send a trigger signal
    gpio_set_level(ULTRASONIC_TRIGGER_PIN, 0);
    ets_delay_us(4);
    gpio_set_level(ULTRASONIC_TRIGGER_PIN, 1);
    ets_delay_us(10);
    gpio_set_level(ULTRASONIC_TRIGGER_PIN, 0);
}

/**
 * @brief Gets the distance measured by the ultrasonic sensor
 *
 * @return float
 */
float ultrasonic_get_distance(void)
{
    return distance;
}

/**
 * @brief ISR for the echo signal of the ultrasonic sensor
 *
 */
void ultrasonic_echo_isr(void *arg)
{

    static uint64_t start;
    static uint64_t end;

    if (gpio_get_level(ULTRASONIC_ECHO_PIN) == 1) // Rising edge
    {
        start = esp_timer_get_time();
    }
    else // Falling edge
    {
        end = esp_timer_get_time();
        distance = (end - start) * 0.0331 / 2;
    }
}

/**
 * @brief Task for measuring the distance with the ultrasonic sensor
 *
 */
void ultrasonic_task(void *pvParameters)
{
    while (1)
    {
        ultrasonic_measure_distance();

        vTaskDelay(pdMS_TO_TICKS(ULTRASONIC_UPDATE_PERIOD_US / 1000));
    }
}

/**
 * @brief Inits the ultrasonic sensor
 */
void ultrasonic_init(void)
{
    // Initialize the ultrasonic sensor
    if (is_init)
    {
        return;
    }

    // Initialize the GPIO pins
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = (1ULL << ULTRASONIC_TRIGGER_PIN);
    io_conf.pull_down_en = 0;
    io_conf.pull_up_en = 0;
    gpio_config(&io_conf);

    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pin_bit_mask = (1ULL << ULTRASONIC_ECHO_PIN);
    io_conf.intr_type = GPIO_INTR_ANYEDGE;
    gpio_config(&io_conf);
    gpio_install_isr_service(ESP_INTR_FLAG_LEVEL1);
    gpio_isr_handler_add(ULTRASONIC_ECHO_PIN, ultrasonic_echo_isr, NULL);

    is_init = true;

    xTaskCreate(ultrasonic_task, "ultrasonic_task", 2048, NULL, 3, &ultrasonic_task_handle);
}
