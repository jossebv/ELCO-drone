/**
 * @file led.h
 * @author Jose Manuel Bravo
 * @brief Header file for the LED component
 * @version 0.1
 * @date 2024-03-19
 *
 * @copyright Copyright (c) 2024
 *
 */

#ifndef LED_H
#define LED_H

#include "fsm.h"

fsm_t *led_fsm_create(uint8_t led_pin);
void led_fsm_set_blinking(fsm_t *fsm);
void led_fsm_set_on(fsm_t *fsm);

void led_init(uint8_t led_pin);
void led_on(uint8_t led_pin, uint8_t *led_status);
void led_off(uint8_t led_pin, uint8_t *led_status);
void led_toggle(uint8_t led_pin, uint8_t *led_status);

#endif