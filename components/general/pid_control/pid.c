/**
 * @file pid.c
 * @author Jose Manuel Bravo
 * @brief Functionality of the PID controller
 * @version 0.1
 * @date 2024-03-04
 *
 * @copyright Copyright (c) 2024
 *
 */

#include <stdlib.h>

#include "pid.h"
#include "esp_timer.h"

/**
 * @brief Create a PID object
 *
 * @param kp Proportional constant
 * @param ki Integral constant
 * @param kd Derivative constant
 * @return pid_data_t* Pointer to the PID object
 */
pid_data_t *pid_create(float kp, float ki, float kd)
{
    pid_data_t *pid = (pid_data_t *)malloc(sizeof(pid_data_t));
    pid->kp = kp;
    pid->ki = ki;
    pid->kd = kd;
    pid->integral = 0;
    pid->last_error = 0;
    pid->last_time = esp_timer_get_time();
    return pid;
}

/**
 * @brief Destroy a PID object
 *
 * @param pid PID object
 */
void pid_destroy(pid_data_t *pid)
{
    free(pid);
}

/**
 * @brief Update the PID controller
 *
 * @param pid PID object
 * @param error Error between the setpoint and the current value
 * @return uint16_t Output of the PID controller. The value should be between a duty cycle.
 */
double pid_update(pid_data_t *pid, float error)
{
    uint64_t current_time = esp_timer_get_time();
    double delta_time = (current_time - pid->last_time) / 1000000.0;
    pid->last_time = current_time;

    pid->integral += error * delta_time;
    float derivative = (error - pid->last_error) / delta_time;
    pid->last_error = error;

    return (double)(pid->kp * error + pid->ki * pid->integral + pid->kd * derivative);
}

/**
 * @brief Reset the PID controller
 *
 * @param pid PID object
 */
void pid_reset(pid_data_t *pid)
{
    pid->last_time = esp_timer_get_time();
    pid->integral = 0;
    pid->last_error = 0;
}

/**
 * @brief Updates a pid_t object with new constants
 *
 * @param pid
 * @param kp
 * @param ki
 * @param kd
 */
void pid_update_constants(pid_data_t *pid, float kp, float ki, float kd)
{
    pid->kp = kp;
    pid->ki = ki;
    pid->kd = kd;
}