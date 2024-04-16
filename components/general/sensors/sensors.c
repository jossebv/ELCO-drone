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
#include "mpu6050.h"
#include "ultrasonic.h"

#include "esp_log.h"

/* DEFINES */
#define DEBUG_SENSORS 1
#define DEBUG_ACCEL 0
#define DEBUG_GYRO 0
#define DEBUG_ACCEL_TO_ANGLES 0

#define RAD_TO_DEG 180 / M_PI

/* TYPEDEFS */

/* FUNCTIONS DECLARATIONS */
double get_altitude_data();
gyro_vector_t get_gyroscope_data();
drone_angles_t gyros_speeds_to_delta_angles(gyro_vector_t gyros_speed, double delta_time_ms);
acc_vector_t get_accelerometer_data();
drone_angles_t acc_to_angles(acc_vector_t accelerations);

/* VARIABLES */
static char *TAG = "sensors";

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

    ESP_LOGI(TAG, "Initializing sensors!!");

    // TODO: INIT ALL THE SENSORS
    mpu6050_init();
    ultrasonic_init();

    is_init = true;
    ESP_LOGI(TAG, "Sensors initialized!!");
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

#if DEBUG_SENSORS
    printf("Drone data: pitch: %f, roll: %f, yaw: %f, altitude: %f\n", drone_data.pitch, drone_data.roll, drone_data.yaw_speed, drone_data.altitude);
#endif

    return drone_data;
}

/**
 * @brief Reads the gyroscope data from the IMU
 *
 * @return gyro_vector_t
 */
gyro_vector_t get_gyroscope_data()
{
    gyro_vector_t gyro_data = mpu6050_read_gyro();
#if DEBUG_GYRO
    printf("Gyroscope data: pitch: %.10f, roll: %.10f, yaw: %.10f\n", gyro_data.pitch, gyro_data.roll, gyro_data.yaw);
#endif
    return gyro_data;
}

/**
 * @brief Reads the accelerometer data from the IMU
 *
 * @return acc_vector_t
 */
acc_vector_t get_accelerometer_data()
{
    acc_vector_t acc_data = mpu6050_read_accelerometer();
#if DEBUG_ACCEL
    printf("Accelerometer data: x: %.10f, y: %.10f, z: %.10f\n", acc_data.x, acc_data.y, acc_data.z);
#endif
    return acc_data;
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
    return ultrasonic_get_distance();
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
    drone_angles.pitch = atan2(accelerations.y, sqrt(pow(accelerations.x, 2) + pow(accelerations.z, 2))) * RAD_TO_DEG;
    drone_angles.roll = atan2(-accelerations.x, sqrt(pow(accelerations.y, 2) + pow(accelerations.z, 2))) * RAD_TO_DEG;

#if DEBUG_ACCEL_TO_ANGLES
    printf("Accel data: x: %f, y: %f, z: %f\n", accelerations.x, accelerations.y, accelerations.z);
    printf("Acc to angles: pitch: %f, roll: %f\n", drone_angles.pitch, drone_angles.roll);
#endif

    return drone_angles;
}