/**
 * @file system.h
 * @author Jose Manuel Bravo
 * @brief system.c header file. Contains all types and methods for system initialization.
 * @version 0.1
 * @date 2024-02-29
 *
 * @copyright Copyright (c) 2024
 *
 */

#ifndef SYSTEM_H
#define SYSTEM_H

#include "fsm.h"
#include "driver/gpio.h"

#define GREEN_LED_PIN 2
#define BLUE_LED_PIN GPIO_NUM_19

void system_task(void *arg);
fsm_t *system_fsm_create(fsm_t *green_led_fsm, fsm_t *blue_led_fsm);

#endif