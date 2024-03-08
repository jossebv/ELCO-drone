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

/* PUBLIC FUNCTIONS */
void motors_init();
void motors_update(command_t command, drone_data_t drone_data)

#endif // MOTORS_H