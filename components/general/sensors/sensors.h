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
void sensors_update_drone_data();

#endif // SENSORS_H