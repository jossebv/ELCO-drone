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
#include <string.h>

// specific
#include "main.h"
#include "sensors.h"
#include "wifi.h"
#include "comb_filter.h"
#include "mpu6050.h"
#include "ultrasonic.h"

#include "esp_log.h"
#include "esp_timer.h"

/* DEFINES */
#define DEBUG_WIFI 0 /**< Debug the data via wifi */

#define DEBUG_SENSORS 0         /**< Debug the sensors data */
#define DEBUG_ACCEL 0           /**< Debug the accelerometer data */
#define DEBUG_GYRO 0            /**< Debug the gyroscope data */
#define DEBUG_ACCEL_TO_ANGLES 0 /**< Debug the acc to angles function */

#define RAD_TO_DEG 180 / M_PI /**< Conversion factor from radians to degrees */

/* TYPEDEFS */

/* FUNCTIONS DECLARATIONS */
double get_altitude_data();
void sensors_read_sensors_data(gyro_vector_t *gyro_data, acc_vector_t *acc_data);
gyro_vector_t get_gyroscope_data();
drone_angles_t gyros_speeds_to_delta_angles(gyro_vector_t gyros_speed, double delta_time_ms);
acc_vector_t get_accelerometer_data();
drone_angles_t acc_to_angles(acc_vector_t accelerations);

/* VARIABLES */
static char *TAG = "sensors";

static bool is_init = false;
static drone_data_t drone_data;
static uint64_t last_update_time = 0;

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
    // ultrasonic_init();

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
    sensors_read_data();
    gyro_vector_t gyros_speeds = get_gyroscope_data();
    acc_vector_t accelerations = get_accelerometer_data();

    uint64_t now = esp_timer_get_time();
    uint64_t delta_time = now - last_update_time;
    last_update_time = now;

    drone_angles_t gyros_delta_angles = gyros_speeds_to_delta_angles(gyros_speeds, delta_time / 1000.0);
    drone_angles_t acc_angles = acc_to_angles(accelerations);

    drone_angles_t drone_angles = comb_filter_get_angles(gyros_delta_angles, acc_angles);
    drone_data.pitch = drone_angles.pitch;
    drone_data.pitch_rate = gyros_speeds.pitch;
    drone_data.roll = drone_angles.roll;
    drone_data.roll_rate = gyros_speeds.roll;

    // Update the yaw speed
    drone_data.yaw_speed = gyros_speeds.yaw;

    // Update the altitude data
    drone_data.altitude = get_altitude_data();

#if DEBUG_SENSORS
    printf("Drone data: time: %lld, pitch: %f, pitch_rate: %f, roll: %f, roll_rate: %f, yaw: %f, altitude: %f\n", esp_timer_get_time(), drone_data.pitch, drone_data.pitch_rate, drone_data.roll, drone_data.roll_rate, drone_data.yaw_speed, drone_data.altitude);
#if DEBUG_WIFI
    static char packet[sizeof(drone_data_t) + 1];
    packet[0] = 0x60;
    memcpy(packet + 1, &drone_data.pitch, sizeof(drone_data.pitch));
    memcpy(packet + 1 + sizeof(drone_data.pitch), &drone_data.roll, sizeof(drone_data.roll));
    wifi_send_data(packet, 17);
#endif
#endif

    return drone_data;
}

drone_data_t sensors_get_drone_data()
{
    return drone_data;
}

void sensors_read_data()
{
    mpu6050_read_data();
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

/* PRIVATE FUNTIONS */
/**
 * @brief Get the altitude data object
 *
 * @return double Altitude data
 */
double get_altitude_data()
{
    // TODO: Connect the ultrasonic sensor
    return 0;
    // return ultrasonic_get_distance();
}

/**
 * @brief Transforms the gyroscope speed into an angle
 *
 * @param gyros_speeds Speeds obtained from the gyroscope
 * @param delta_time_ms Time between samples in miliseconds
 * @return double
 */
drone_angles_t gyros_speeds_to_delta_angles(gyro_vector_t gyros_speeds, double delta_time_ms)
{
    drone_angles_t delta_angles;
    delta_angles.pitch = -gyros_speeds.pitch * delta_time_ms / 1000;
    delta_angles.roll = -gyros_speeds.roll * delta_time_ms / 1000;
    return delta_angles;
}

/**
 * @brief Converts the accelerations into angles
 *
 * @param accelerations acc_vector_t with the accelerations
 * @return drone_angles_t drone angles obtained from the accelerations
 */
drone_angles_t acc_to_angles(acc_vector_t accelerations)
{
    drone_angles_t drone_angles;
    drone_angles.pitch = -atan2(accelerations.y, sqrt(pow(accelerations.x, 2) + pow(accelerations.z, 2))) * RAD_TO_DEG;
    drone_angles.roll = -atan2(-accelerations.x, sqrt(pow(accelerations.y, 2) + pow(accelerations.z, 2))) * RAD_TO_DEG;

#if DEBUG_ACCEL_TO_ANGLES
    printf("Accel data: x: %f, y: %f, z: %f\n", accelerations.x, accelerations.y, accelerations.z);
    printf("Acc to angles: pitch: %f, roll: %f\n", drone_angles.pitch, drone_angles.roll);
#endif

    return drone_angles;
}