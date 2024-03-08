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
#include <stdio.h>

// specific
#include "main.h"
#include "sensors.h"
#include "comb_filter.h"

/* DEFINES */
#define DEBUG_SENSORS 1

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
static drone_data_t drone_data;

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
 * @brief Gets the data from the sensors and updates the drone data
 *
 * @return drone_data_t
 */
drone_data_t sensors_update_drone_data()
{

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

    // Update the altitude data
    drone_data.altitude = get_altitude_data();

    return drone_data;

#if DEBUG_SENSORS
    printf("Drone data: pitch: %f, roll: %f, yaw: %f, altitude: %f\n", drone_data.pitch, drone_data.roll, drone_data.yaw_speed, drone_data.altitude);
#endif
}

/**
 * @brief Reads the gyroscope data from the IMU
 *
 * @return gyro_vector_t
 */
gyro_vector_t get_gyroscope_data()
{
    return mpu6050_read_gyro();
}

/**
 * @brief Reads the accelerometer data from the IMU
 *
 * @return acc_vector_t
 */
acc_vector_t get_accelerometer_data()
{
    return mpu6050_read_accelerometer();
}

/**
 * @brief Calibrate the IMU
 *
 */
void sensors_calibrate_imu(gyro_vector_t gyro_offsets, acc_vector_t acc_offsets)
{
    mpu6050_calibrate(gyro_offsets, acc_offsets);
}

/**
 * @brief Get the drone data object containing the pitch, roll, yaw speed and altitude
 *
 * @return drone_data_t
 */
drone_data_t sensors_get_drone_data()
{
    return drone_data;
}

/* PRIVATE FUNTIONS */
double get_altitude_data()
{
    // TODO: CALL THE ALTITUDE SENSORS TO GET THE ALTITUDE
    return 0;
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

drone_angles_t acc_to_angles(acc_vector_t accelerations)
{
    drone_angles_t drone_angles;
    drone_angles.pitch = atan2(-accelerations.x, sqrt(pow(accelerations.y, 2) + pow(accelerations.z, 2))) * RAD_TO_DEG;
    drone_angles.roll = atan2(accelerations.y, accelerations.z) * RAD_TO_DEG;

    return drone_angles;
}