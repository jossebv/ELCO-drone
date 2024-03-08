/**
 * @file mpu6050.c
 * @author Jose Manuel Bravo
 * @brief Driver for the MPU6050 sensor
 * @version 0.1
 * @date 2024-02-29
 *
 * @copyright Copyright (c) 2024
 *
 */

/* INCLUDES */
#include <stdint.h>
#include <stdbool.h>

#include "mpu6050.h"
#include "driver/i2c.h"
#include "sdkconfig.h"

/* DEFINES */
#define MPU6050_ADDR 0x68
#define MPU6050_GYRO_CONFIG_REG 0x1B
#define MPU6050_GYRO_XOUT_H_REG 0x43
#define MPU6050_ACCEL_CONFIG_REG 0x1C
#define MPU6050_ACCEL_XOUT_H_REG 0x3B
#define MPU6050_PWR_MGMT_1_REG 0x6B
#define MPU6050_SMPLRT_DIV_REG 0x19
#define MPU6050_CONFIG_REG 0x1A

#define PWR_MGMT_1_DEVICE_RESET_MASK 0X1 << 7
#define PWR_MGMT_1_DEVICE_SLEEP_MASK 0X1 << 6
#define GYRO_CONFIG_NO_TEST_FS_2000 0x18
#define ACCEL_CONFIG_NO_TEST_FS_2G 0x00

/* VARIABLES */
static bool is_init = false;
static double gyro_offset_pitch, gyro_offset_roll, gyro_offset_yaw;
static double accel_offset_x, accel_offset_y, accel_offset_z;

/* FUNCTIONS DECLARATIONS */
void mpu6050_reset_offsets();
void reset_device();
void wait_for_reset();
void mpu6050_wake_up();
void select_clk_source();
void set_gyro_range();
void set_accelerometer_range();
void set_sample_rate();
void configure_low_pass_filter();

/* PUBLIC FUNCTIONS */

/**
 * @brief Inits the MPU6050 sensor
 *
 */
void mpu6050_init()
{
    if (is_init)
    {
        return;
    }

    mpu6050_reset_offsets();

    reset_device();
    wait_for_reset();

    mpu6050_wake_up();
    vTaskDelay(pdMS_TO_TICKS(100)); // Wait for the device to wake up and stabilize clock
    select_clk_source();

    set_gyro_range();
    set_accelerometer_range();
    set_sample_rate();
    configure_low_pass_filter();

    is_init = true;
}

/**
 * @brief Reads the gyro data
 *
 * @return gyro_vector_t
 */
gyro_vector_t mpu6050_read_gyro()
{
    gyro_vector_t gyro;
    uint8_t read_buffer[6]; // 2 bytes for each axis
    uint8_t write_reg = MPU6050_GYRO_XOUT_H_REG;
    i2c_master_write_read_device(I2C_NUM_0, MPU6050_ADDR, &write_reg, sizeof(write_reg), read_buffer, sizeof(read_buffer), pdMS_TO_TICKS(1000));

    gyro.pitch = ((int8_t)read_buffer[0] << 8 | (int8_t)read_buffer[1]) - gyro_offset_pitch; // gyroscope x axis
    gyro.roll = ((int8_t)read_buffer[2] << 8 | (int8_t)read_buffer[3]) - gyro_offset_roll;   // gyroscope y axis
    gyro.yaw = ((int8_t)read_buffer[4] << 8 | (int8_t)read_buffer[5]) - gyro_offset_yaw;     // gyroscope z axis
    return gyro;
}

/**
 * @brief Reads the accelerometer data
 *
 * @return acc_vector_t
 */
acc_vector_t mpu6050_read_accelerometer()
{
    acc_vector_t acc;
    uint8_t read_buffer[6]; // 2 bytes for each axis
    uint8_t write_reg = MPU6050_ACCEL_XOUT_H_REG;
    i2c_master_write_read_device(I2C_NUM_0, MPU6050_ADDR, &write_reg, sizeof(write_reg), read_buffer, sizeof(read_buffer), pdMS_TO_TICKS(1000));

    acc.x = ((int8_t)read_buffer[0] << 8 | (int8_t)read_buffer[1]) - accel_offset_x; // accelerometer x axis
    acc.y = ((int8_t)read_buffer[2] << 8 | (int8_t)read_buffer[3]) - accel_offset_y; // accelerometer y axis
    acc.z = ((int8_t)read_buffer[4] << 8 | (int8_t)read_buffer[5]) - accel_offset_z; // accelerometer z axis
    return acc;
}

/* PRIVATE FUNCTIONS */

/**
 * @brief Resets the offsets
 *
 */
void mpu6050_reset_offsets()
{
    gyro_offset_pitch = 0;
    gyro_offset_roll = 0;
    gyro_offset_yaw = 0;
    accel_offset_x = 0;
    accel_offset_y = 0;
    accel_offset_z = 0;
}

/**
 * @brief Calibrates the sensor adding the offsets to the current values
 *
 * Is divided by 2 to help the convergence of the calibration
 *
 * @param gyro_offsets
 * @param acc_offsets
 */
void mpu6050_calibrate(gyro_vector_t gyro_offsets, acc_vector_t acc_offsets)
{
    gyro_offset_pitch += (gyro_offsets.pitch / 2);
    gyro_offset_roll += (gyro_offsets.roll / 2);
    gyro_offset_yaw += (gyro_offsets.yaw / 2);
    accel_offset_x += (acc_offsets.x / 2);
    accel_offset_y += (acc_offsets.y / 2);
    accel_offset_z += ((acc_offsets.z - 1) / 2); // Gravity is 1g
}

/**
 * @brief Resets the device
 *
 */
void reset_device()
{
    uint8_t write_buffer[2] = {MPU6050_PWR_MGMT_1_REG, 0xff & PWR_MGMT_1_DEVICE_RESET_MASK};
    i2c_master_write_to_device(I2C_NUM_0, MPU6050_ADDR, write_buffer, sizeof(write_buffer), pdMS_TO_TICKS(1000));
}

/**
 * @brief Waits for the device to reset
 *
 */
void wait_for_reset()
{
    uint8_t write_reg = MPU6050_PWR_MGMT_1_REG;
    uint8_t read_buffer;
    do
    {
        vTaskDelay(pdMS_TO_TICKS(100));
        i2c_master_write_read_device(I2C_NUM_0, MPU6050_ADDR, &write_reg, sizeof(write_reg), &read_buffer, sizeof(read_buffer), pdMS_TO_TICKS(1000));
    } while (read_buffer & PWR_MGMT_1_DEVICE_RESET_MASK);
}

/**
 * @brief Wakes up the device
 *
 */
void mpu6050_wake_up()
{
    uint8_t write_buffer[2] = {MPU6050_PWR_MGMT_1_REG, 0x00};
    i2c_master_write_to_device(I2C_NUM_0, MPU6050_ADDR, write_buffer, sizeof(write_buffer), pdMS_TO_TICKS(1000));
}

/**
 * @brief Selects the clock source
 *
 */
void select_clk_source()
{
    uint8_t write_buffer[2] = {MPU6050_PWR_MGMT_1_REG, 0x01};
    i2c_master_write_to_device(I2C_NUM_0, MPU6050_ADDR, write_buffer, sizeof(write_buffer), pdMS_TO_TICKS(1000));
}

/**
 * @brief Set the gyro range object
 *
 */
void set_gyro_range()
{
    uint8_t write_buffer[2] = {MPU6050_GYRO_CONFIG_REG, GYRO_CONFIG_NO_TEST_FS_2000};
    i2c_master_write_to_device(I2C_NUM_0, MPU6050_ADDR, write_buffer, sizeof(write_buffer), pdMS_TO_TICKS(1000));
}

/**
 * @brief Set the accelerometer range object
 *
 */
void set_accelerometer_range()
{
    uint8_t write_buffer[2] = {MPU6050_ACCEL_CONFIG_REG, ACCEL_CONFIG_NO_TEST_FS_2G};
    i2c_master_write_to_device(I2C_NUM_0, MPU6050_ADDR, write_buffer, sizeof(write_buffer), pdMS_TO_TICKS(1000));
}

/**
 * @brief Set the sample rate object
 *
 */
void set_sample_rate()
{
    uint8_t write_buffer[2] = {MPU6050_SMPLRT_DIV_REG, 0x07};
    i2c_master_write_to_device(I2C_NUM_0, MPU6050_ADDR, write_buffer, sizeof(write_buffer), pdMS_TO_TICKS(1000));
}

/**
 * @brief Configures the low pass filter
 *
 */
void configure_low_pass_filter()
{
    uint8_t write_buffer[2] = {MPU6050_CONFIG_REG, 0x06};
    i2c_master_write_to_device(I2C_NUM_0, MPU6050_ADDR, write_buffer, sizeof(write_buffer), pdMS_TO_TICKS(1000));
}