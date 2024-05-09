/**
 * @file mpu6050.h
 * @author Jose Manuel Bravo Pacheco
 * @brief Header file for the MPU6050 driver
 * @version 0.1
 * @date 2024-02-29
 *
 * @copyright Copyright (c) 2024
 *
 */

#ifndef MPU6050_H
#define MPU6050_H

/* TYPEDEFS */

/**
 * @brief Vector with the information obtained from the gyroscope
 *
 */
typedef struct gyro_vector_t
{
    double pitch; /**< pitch is the rotation around the x-axis */
    double roll;  /**< roll is the rotation around the y-axis*/
    double yaw;   /**< yaw is the rotation around the z-axis*/
} gyro_vector_t;

/**
 * @brief Vector with the information obtained from the accelerometer
 *
 */
typedef struct acc_vector_t
{
    double x; /**< x is the acceleration in the x-axis */
    double y; /**< y is the acceleration in the y-axis */
    double z; /**< z is the acceleration in the z-axis */
} acc_vector_t;

/* PUBLIC FUNCTIONS */
void mpu6050_init();
void mpu6050_calibrate(gyro_vector_t gyro_offsets, acc_vector_t acc_offsets);
gyro_vector_t mpu6050_read_gyro();
acc_vector_t mpu6050_read_accelerometer();

#endif // MPU6050_H