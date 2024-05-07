/**
 * @file motors.h
 * @author Jose Manuel Bravo
 * @brief Header file for the motors component.
 * @version 0.1
 * @date 2024-02-29
 *
 * @copyright Copyright (c) 2024
 *
 */

#ifndef MOTORS_H
#define MOTORS_H

/* INCLUDES */
#include <stdbool.h>

#include "controller.h"
#include "sensors.h"

/* PUBLIC FUNCTIONS */
void motors_init();
void motors_update(command_t command, drone_data_t drone_data);
void motors_reset();
bool motors_update_pid_constants(uint8_t pid_number, float kp, float ki, float kd);

#endif // MOTORS_H