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

/**
 * @brief Inits the gpio for the led
 *
 */
void led_init(uint8_t led_pin)
{

    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << led_pin),
        .mode = GPIO_MODE_OUTPUT,
        .intr_type = GPIO_INTR_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .pull_up_en = GPIO_PULLUP_DISABLE};

    gpio_config(&io_conf);
}

/**
 * @brief Turns on the led
 *
 */
void led_on(uint8_t led_pin, uint8_t *led_status)
{
    *led_status = 1;
    gpio_set_level(led_pin, *led_status);
}

/**
 * @brief Turns off the led
 *
 */
void led_off(uint8_t led_pin, uint8_t *led_status)
{
    *led_status = 0;
    gpio_set_level(led_pin, *led_status);
}

/**
 * @brief Alternates the led status
 *
 */
void led_toggle(uint8_t led_pin, uint8_t *led_status)
{
    *led_status = !*led_status;
    gpio_set_level(led_pin, *led_status);
}