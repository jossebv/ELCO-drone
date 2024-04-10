/**
 * @file controller.h
 * @author Jose Manuel Bravo
 * @brief Header file for the remote controller file
 * @version 0.1
 * @date 2024-03-06
 *
 * @copyright Copyright (c) 2024
 *
 */

#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <stdint.h>

/**
 * @brief Command structure. Contains the pitch, roll, yaw and thrust
 *
 */
typedef struct command_t
{
    float pitch;     // Angle in degrees
    float roll;      // Angle in degrees
    float yaw_speed; // Rotation speed in degrees per second
    uint16_t thrust; // Thrust in percentage
} command_t;

void controller_get_command(command_t *command);

#endif // CONTROLLER_H