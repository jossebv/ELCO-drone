/**
 * @file motors.c
 * @author Jose Manuel Bravo
 * @brief Contains the functionality for controlling the drone motors.
 * @version 0.1
 * @date 2024-02-29
 *
 * @copyright Copyright (c) 2024
 *
 */

/* INCLUDES */
#include "motors.h"
#include "pid.h"

/* DEFINES */
#define PITCH_KP 1
#define PITCH_KI 0
#define PITCH_KD 0

#define ROLL_KP 1
#define ROLL_KI 0
#define ROLL_KD 0

#define YAW_KP 1
#define YAW_KI 0
#define YAW_KD 0

/* VARIABLES */
static bool is_init = false;
static pid_data_t *pid_pitch, *pid_roll, *pid_yaw;

/* FUNCTIONS DECLARATIONS */

/* PUBLIC FUNCTIONS */
/**
 * @brief Inits all the motors
 *
 */
void motors_init()
{
    // TODO: Implement this function
    is_init = true;

    // Initialize the PID controllers
    pid_pitch = pid_create(PITCH_KP, PITCH_KI, PITCH_KD);
    pid_roll = pid_create(ROLL_KP, ROLL_KI, ROLL_KD);
    pid_yaw = pid_create(YAW_KP, YAW_KI, YAW_KD);
}