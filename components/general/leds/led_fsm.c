/**
 * @file led_fsm.c
 * @author Jose Manuel Bravo
 * @brief FSM for the led
 * @version 0.1
 * @date 2024-03-19
 *
 * @copyright Copyright (c) 2024
 *
 */

#include <stdlib.h>

#include "esp_timer.h"

#include "led.h"

#define LED_BLINKING_PERIOD_US 500000

typedef enum led_fsm_states
{
    BLINKING = 0,
    ON,
    OFF
} led_fsm_states_t;

typedef struct fsm_led_t
{
    fsm_t fsm;
    uint32_t next;
    uint8_t led_pin;
    uint8_t led_status;
} fsm_led_t;

/**
 * @brief Sets the fsm to blink
 *
 * @param fsm
 */
void led_fsm_set_blinking(fsm_t *fsm)
{
    fsm_led_t *led_fsm = (fsm_led_t *)fsm;
    fsm->current_state = BLINKING;
    led_fsm->next = esp_timer_get_time() + LED_BLINKING_PERIOD_US;
    led_toggle(led_fsm->led_pin, &led_fsm->led_status);
}

/**
 * @brief Sets the fsm to on
 *
 * @param fsm
 */
void led_fsm_set_on(fsm_t *fsm)
{
    fsm_led_t *led_fsm = (fsm_led_t *)fsm;
    fsm->current_state = ON;
    led_on(led_fsm->led_pin, &led_fsm->led_status);
}

/**
 * @brief Checks if the time has elapsed
 *
 * @param fsm
 * @return int
 */
int is_time_elapsed(fsm_t *fsm)
{
    fsm_led_t *led_fsm = (fsm_led_t *)fsm;
    return led_fsm->next < esp_timer_get_time();
}

/**
 * @brief Checks if the led is on
 *
 * @param fsm
 * @return int
 */
int is_on(fsm_t *fsm)
{
    return fsm->current_state == ON;
}

/**
 * @brief Toggles the led
 *
 * @param fsm
 */
void do_toggle_led(fsm_t *fsm)
{
    fsm_led_t *led_fsm = (fsm_led_t *)fsm;
    led_fsm->next = esp_timer_get_time() + LED_BLINKING_PERIOD_US;
    led_toggle(led_fsm->led_pin, &led_fsm->led_status);
}

/**
 * @brief Turns the led on
 *
 * @param fsm
 */
void do_turn_led_on(fsm_t *fsm)
{
    fsm_led_t *led_fsm = (fsm_led_t *)fsm;
    led_on(led_fsm->led_pin, &led_fsm->led_status);
}

/**
 * @brief Initializes the led fsm
 *
 * @param fsm
 */
void led_fsm_init(fsm_t *fsm, uint8_t led_pin)
{
    fsm_led_t *led_fsm = (fsm_led_t *)fsm;

    static fsm_trans_t system_fsm_tt[] = {
        {BLINKING, is_time_elapsed, BLINKING, do_toggle_led},
        {ON, is_on, ON, do_turn_led_on},
        {-1, NULL, -1, NULL}};

    led_init(led_pin);
    fsm_init(fsm, system_fsm_tt);
    led_fsm->led_pin = led_pin;
    led_fsm->led_status = 0;
    led_fsm->next = esp_timer_get_time() + LED_BLINKING_PERIOD_US;
}

/**
 * @brief Creates the led fsm
 *
 * @return fsm_t*
 */
fsm_t *led_fsm_create(uint8_t led_pin)
{
    fsm_t *fsm = (fsm_t *)malloc(sizeof(fsm_led_t));
    led_fsm_init(fsm, led_pin);

    return fsm;
}

/**
 * @brief Destroys the led fsm
 *
 * @param fsm
 */
void led_fsm_destroy(fsm_t *fsm)
{
    free(fsm);
}
