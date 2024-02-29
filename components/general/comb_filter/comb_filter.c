/**
 * @file comb_filter.c
 * @author Jose Manuel Bravo
 * @brief Obtains the calculation for the combinatory filter
 * @version 0.1
 * @date 2024-02-28
 *
 * @copyright Copyright (c) 2024
 *
 */

/* INCLUDES */
#include "comb_filter.h"

/* STATIC VARIABLES */
static double prev_pitch;
static double prev_roll;

/* FUNCTIONS DECLARATIONS */
static double update_angle(double gyros_delta_angle, double acc_angle, double *angle_to_update);

/* PUBLIC FUNCTIONS */
void comb_filter_init()
{
    prev_pitch = 0;
    prev_roll = 0;
}

drone_angles_t comb_filter_get_angles(drone_angles_t gyros_delta_angle, drone_angles_t acc_angle)
{
    drone_angles_t drone_angles;
    drone_angles.pitch = update_angle(gyros_delta_angle.pitch, acc_angle.pitch, &prev_pitch);
    drone_angles.roll = update_angle(gyros_delta_angle.roll, acc_angle.roll, &prev_roll);

    return drone_angles;
}

/* PRIVATE FUNCTIONS */
static double update_angle(double gyros_delta_angle, double acc_angle, double *angle_to_update)
{
    double angle_gyro = (gyros_delta_angle + *angle_to_update) * 0.98;
    double angle_acc = acc_angle * 0.02;
    double angle = angle_gyro + angle_acc;
    *angle_to_update = angle;
    return angle;
}
