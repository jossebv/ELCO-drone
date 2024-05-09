/**
 * @file pid.h
 * @author Jose Manuel Bravo
 * @brief Header file for PID controller
 * @version 0.1
 * @date 2024-03-04
 *
 * @copyright Copyright (c) 2024
 *
 */

#ifndef PID_H
#define PID_H

/**
 * @brief Structure for PID controller, contains the constants and saved values for the pid control.
 *
 */
typedef struct pid_data_t
{
    double kp;         /**< PID proportional constants */
    double ki;         /**< PID integral constants */
    double kd;         /**< PID derivative constants */
    double integral;   /**< Integral value */
    double last_error; /**< Last error value */
    double last_time;  /**< Last time PID was updated*/
} pid_data_t;

pid_data_t *pid_create(float kp, float ki, float kd);
void pid_destroy(pid_data_t *pid);
double pid_update(pid_data_t *pid, float error);
void pid_reset(pid_data_t *pid);
void pid_update_constants(pid_data_t *pid, float kp, float ki, float kd);

#endif // PID_H