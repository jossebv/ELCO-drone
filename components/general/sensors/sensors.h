/**
 * @file sensors.h
 * @author Jose Manuel Bravo
 * @brief Header of the sensors.c file. Contains all types and methods for sensor readings.
 * @version 0.1
 * @date 2024-02-28
 *
 * @copyright Copyright (c) 2024
 *
 */
#ifndef SENSORS_H
#define SENSORS_H

#include "mpu6050.h"

/**
 * @brief Struct with the variables needed for controlling the drone
 *
 */
typedef struct drone_data_t
{
    double altitude;
    double pitch;
    double roll;
    double yaw_speed;
} drone_data_t;

void sensors_init();
drone_data_t sensors_update_drone_data();
drone_data_t sensors_get_drone_data();
gyro_vector_t get_gyroscope_data();
acc_vector_t get_accelerometer_data();
void sensors_calibrate_imu(gyro_vector_t gyro_offsets, acc_vector_t acc_offsets);

#endif // SENSORS_H