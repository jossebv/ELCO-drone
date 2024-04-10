/**
 * @file led.c
 * @author Jose Manuel Bravo
 * @brief Led driver
 * @version 0.1
 * @date 2024-03-19
 *
 * @copyright Copyright (c) 2024
 *
 */

#include <stdint.h>
#include <stdbool.h>

#include "led.h"
#include "driver/gpio.h"

#define LED_PIN GPIO_NUM_2

static uint8_t led_status = 0;
static bool is_init = false;

/**
 * @brief Inits the gpio for the led
 *
 */
void led_init(void)
{
    if (is_init)
    {
        return;
    }

    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << LED_PIN),
        .mode = GPIO_MODE_OUTPUT,
        .intr_type = GPIO_INTR_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .pull_up_en = GPIO_PULLUP_DISABLE};

    gpio_config(&io_conf);

    is_init = true;
}

/**
 * @brief Turns on the led
 *
 */
void led_on(void)
{
    led_status = 1;
    gpio_set_level(LED_PIN, led_status);
}

/**
 * @brief Turns off the led
 *
 */
void led_off(void)
{
    led_status = 0;
    gpio_set_level(LED_PIN, led_status);
}

/**
 * @brief Alternates the led status
 *
 */
void led_toggle(void)
{
    led_status = !led_status;
    gpio_set_level(LED_PIN, led_status);
}