/**
 * @file sensors.c
 * @author Jose Manuel Bravo
 * @brief Contains the functionality for reading and processing all drone sensors.
 * @version 0.1
 * @date 2024-02-28
 *
 * @copyright Copyright (c) 2024
 *
 */

/* INCLUDES */
// general
#include <math.h>
#include <stdbool.h>

// specific
#include "main.h"
#include "sensors.h"
#include "mpu6050.h"
#include "comb_filter.h"

/* DEFINES */
#define RAD_TO_DEG 180 / M_PI

/* TYPEDEFS */

/* FUNCTIONS DECLARATIONS */
double get_altitude_data();
gyro_vector_t get_gyroscope_data();
drone_angles_t gyros_speeds_to_delta_angles(gyro_vector_t gyros_speed, double delta_time_ms);
acc_vector_t get_accelerometer_data();
drone_angles_t acc_to_angles(acc_vector_t accelerations);

/* VARIABLES */
static bool is_init = false;

/* PUBLIC FUNCTIONS */

/**
 * @brief Inits all the sensors
 *
 */
void sensors_init()
{
    if (is_init)
    {
        return;
    }

    // TODO: INIT ALL THE SENSORS
    mpu6050_init();

    is_init = true;
}

/**
 * @brief Returns all the data obtained from the sensors
 *
 * @return drone_data_t
 */
drone_data_t sensors_get_drone_data()
{
    // Update the altitude data
    drone_data_t drone_data;
    drone_data.altitude = get_altitude_data();

    // Update the pitch and roll data
    gyro_vector_t gyros_speeds = get_gyroscope_data();
    acc_vector_t accelerations = get_accelerometer_data();

    drone_angles_t gyros_delta_angles = gyros_speeds_to_delta_angles(gyros_speeds, DRONE_UPDATE_MS);
    drone_angles_t acc_angles = acc_to_angles(accelerations);

    drone_angles_t drone_angles = comb_filter_get_angles(gyros_delta_angles, acc_angles);
    drone_data.pitch = drone_angles.pitch;
    drone_data.roll = drone_angles.roll;

    // Update the yaw speed
    drone_data.yaw_speed = gyros_speeds.yaw;

    return drone_data;
}

/* PRIVATE FUNTIONS */
double get_altitude_data()
{
    // TODO: CALL THE ALTITUDE SENSORS TO GET THE ALTITUDE
    return 0;
}

gyro_vector_t get_gyroscope_data()
{
    // TODO: CALL THE MPU6050 DRIVER TO OBTAIN THE SPEEDS FROM THE GYROSCOPES
    gyro_vector_t gyros_vector;
    gyros_vector.pitch = 0;
    gyros_vector.roll = 0;
    gyros_vector.yaw = 0;
    return gyros_vector;
}

/**
 * @brief Transforms the gyroscope speed into an angle
 *
 * @param gyros_speed Speed obtained from the gyroscope
 * @param delta_time_ms Time between samples in miliseconds
 * @return double
 */
drone_angles_t gyros_speeds_to_delta_angles(gyro_vector_t gyros_speeds, double delta_time_ms)
{
    drone_angles_t delta_angles;
    delta_angles.pitch = gyros_speeds.pitch * delta_time_ms / 1000;
    delta_angles.roll = gyros_speeds.roll * delta_time_ms / 1000;
    return delta_angles;
}

acc_vector_t get_accelerometer_data()
{
    // TODO: Call the MPU6050 drivers to obtain the accelerations in the three axes
    acc_vector_t acc_vector;
    acc_vector.x = 0;
    acc_vector.y = 0;
    acc_vector.z = 0;
    return acc_vector;
}

drone_angles_t acc_to_angles(acc_vector_t accelerations)
{
    drone_angles_t drone_angles;
    drone_angles.pitch = atan2(-accelerations.x, sqrt(pow(accelerations.y, 2) + pow(accelerations.z, 2))) * RAD_TO_DEG;
    drone_angles.roll = atan2(accelerations.y, accelerations.z) * RAD_TO_DEG;

    return drone_angles;
}