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
#include <math.h>

#include "mpu6050.h"
#include "driver/i2c.h"
#include "sdkconfig.h"

/* DEFINES */
#define MPU6050_ADDR 0x68             /**< Address of the MPU6050 sensor */
#define MPU6050_GYRO_CONFIG_REG 0x1B  /**< Address of GYRO_CONFIG register */
#define MPU6050_GYRO_XOUT_H_REG 0x43  /**< Address of GYRO_XOUT_H register */
#define MPU6050_ACCEL_CONFIG_REG 0x1C /**< Address of ACCEL_CONFIG register */
#define MPU6050_ACCEL_XOUT_H_REG 0x3B /**< Address of ACCEL_XOUT_H register */
#define MPU6050_PWR_MGMT_1_REG 0x6B   /**< Address of PWR_MGMT_1 register */
#define MPU6050_SMPLRT_DIV_REG 0x19   /**< Address of SMPLRT_DIV register */
#define MPU6050_CONFIG_REG 0x1A       /**< Address of CONFIG register */

#define PWR_MGMT_1_DEVICE_RESET_MASK 0X1 << 7 /**< Reset device */
#define PWR_MGMT_1_DEVICE_SLEEP_MASK 0X1 << 6 /**< Sleep mode */
#define GYRO_CONFIG_NO_TEST_FS_2000 0x18      /**< Mask for full scale of 2000 degrees per second on gyroscopes*/
#define ACCEL_CONFIG_NO_TEST_FS_2G 0x00       /**< Mask for full scale of 2G on accelerometers*/

/* VARIABLES */
static bool is_init = false;

static gyro_vector_t gyro_mem;
static acc_vector_t acc_mem;

static gyro_vector_t gyro_data;
static acc_vector_t acc_data;

static double gyro_offset_pitch, gyro_offset_roll, gyro_offset_yaw;
static double accel_offset_x, accel_offset_y, accel_offset_z = 0;

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

void mpu6050_read_data()
{
    uint8_t read_buffer[14]; // 2 bytes for each axis
    uint8_t write_reg = MPU6050_ACCEL_XOUT_H_REG;

    esp_err_t ret = i2c_master_write_read_device(I2C_NUM_0, MPU6050_ADDR, &write_reg, sizeof(write_reg), read_buffer, sizeof(read_buffer), pdMS_TO_TICKS(10));

    if (ret != ESP_OK)
    {
        printf("Error reading data\n");
        return;
    }

    int16_t acc_x = (int16_t)((uint8_t)read_buffer[0] << 8 | (uint8_t)read_buffer[1]);
    int16_t acc_y = (int16_t)((uint8_t)read_buffer[2] << 8 | (uint8_t)read_buffer[3]);
    int16_t acc_z = (int16_t)((uint8_t)read_buffer[4] << 8 | (uint8_t)read_buffer[5]);
    int16_t gyro_pitch = (int16_t)((uint8_t)read_buffer[8] << 8 | (uint8_t)read_buffer[9]);
    int16_t gyro_roll = (int16_t)((uint8_t)read_buffer[10] << 8 | (uint8_t)read_buffer[11]);
    int16_t gyro_yaw = (int16_t)((uint8_t)read_buffer[12] << 8 | (uint8_t)read_buffer[13]);

    acc_data.x = ((double)acc_x / 16384.0);        // accelerometer x axis
    acc_data.y = ((double)acc_y / 16384.0);        // accelerometer y axis
    acc_data.z = ((double)acc_z / 16384.0);        // accelerometer z axis
    gyro_data.pitch = ((double)gyro_pitch / 16.4); // gyroscope x axis
    gyro_data.roll = ((double)gyro_roll / 16.4);   // gyroscope y axis
    gyro_data.yaw = ((double)gyro_yaw / 16.4);     // gyroscope z axis

    acc_data.x -= accel_offset_x;
    acc_data.y -= accel_offset_y;
    acc_data.z -= accel_offset_z;
    gyro_data.pitch -= gyro_offset_pitch;
    gyro_data.roll -= gyro_offset_roll;
    gyro_data.yaw -= gyro_offset_yaw;
}

/**
 * @brief Reads the gyro data
 *
 * @return gyro_vector_t
 */
gyro_vector_t mpu6050_read_gyro()
{
    return gyro_data;
    // gyro_vector_t gyro;
    // uint8_t read_buffer[6]; // 2 bytes for each axis
    // uint8_t write_reg = MPU6050_GYRO_XOUT_H_REG;
    // esp_err_t err = i2c_master_write_read_device(I2C_NUM_0, MPU6050_ADDR, &write_reg, sizeof(write_reg), read_buffer, sizeof(read_buffer), pdMS_TO_TICKS(10));

    // if (err != ESP_OK)
    // {
    //     printf("Error reading gyro data\n");
    //     return gyro_mem;
    // }

    // int16_t gyro_x = (int16_t)((uint8_t)read_buffer[0] << 8 | (uint8_t)read_buffer[1]);
    // int16_t gyro_y = (int16_t)((uint8_t)read_buffer[2] << 8 | (uint8_t)read_buffer[3]);
    // int16_t gyro_z = (int16_t)((uint8_t)read_buffer[4] << 8 | (uint8_t)read_buffer[5]);

    // gyro.pitch = ((double)gyro_x / 16.4); // gyroscope x axis
    // gyro.roll = ((double)gyro_y / 16.4);  // gyroscope y axis
    // gyro.yaw = ((double)gyro_z / 16.4);   // gyroscope z axis

    // gyro.pitch -= gyro_offset_pitch;
    // gyro.roll -= gyro_offset_roll;
    // gyro.yaw -= gyro_offset_yaw;

    // gyro_mem = gyro;

    // return gyro;
}

/**
 * @brief Reads the accelerometer data
 *
 * @return acc_vector_t
 */
acc_vector_t mpu6050_read_accelerometer()
{
    return acc_data;
    // acc_vector_t acc;
    // uint8_t read_buffer[6]; // 2 bytes for each axis
    // uint8_t write_reg = MPU6050_ACCEL_XOUT_H_REG;
    // esp_err_t err = i2c_master_write_read_device(I2C_NUM_0, MPU6050_ADDR, &write_reg, sizeof(write_reg), read_buffer, sizeof(read_buffer), pdMS_TO_TICKS(10));

    // if (err != ESP_OK)
    // {
    //     printf("Error reading accelerometer data\n");
    //     return acc_mem;
    // }

    // int16_t acc_x = (int16_t)((uint8_t)read_buffer[0] << 8 | (uint8_t)read_buffer[1]);
    // int16_t acc_y = (int16_t)((uint8_t)read_buffer[2] << 8 | (uint8_t)read_buffer[3]);
    // int16_t acc_z = (int16_t)((uint8_t)read_buffer[4] << 8 | (uint8_t)read_buffer[5]);

    // acc.x = ((double)acc_x / 16384.0); // accelerometer x axis
    // acc.y = ((double)acc_y / 16384.0); // accelerometer y axis
    // acc.z = ((double)acc_z / 16384.0); // accelerometer z axis

    // acc.x -= accel_offset_x;
    // acc.y -= accel_offset_y;
    // acc.z -= accel_offset_z;

    // acc_mem = acc;

    // return acc;
}

/* PRIVATE FUNCTIONS */

/**
 * @brief Resets the offsets
 *
 */
void mpu6050_reset_offsets()
{
    gyro_offset_pitch = 0.0;
    gyro_offset_roll = 0.0;
    gyro_offset_yaw = 0.0;
    accel_offset_x = 0.0;
    accel_offset_y = 0.0;
    accel_offset_z = 0.0;
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
    // printf("Calibrating: %f %f %f %f %f %f\n", gyro_offsets.pitch, gyro_offsets.roll, gyro_offsets.yaw, acc_offsets.x, acc_offsets.y, acc_offsets.z);
    if (isinf(gyro_offsets.pitch) || isinf(gyro_offsets.roll) || isinf(gyro_offsets.yaw) || isinf(acc_offsets.x) || isinf(acc_offsets.y) || isinf(acc_offsets.z))
    {
        // printf("Calibration failed\n");
        return;
    }

    gyro_offset_pitch += (gyro_offsets.pitch);
    gyro_offset_roll += (gyro_offsets.roll);
    gyro_offset_yaw += (gyro_offsets.yaw);
    accel_offset_x += (acc_offsets.x);
    accel_offset_y += (acc_offsets.y);
    accel_offset_z += ((acc_offsets.z - 1)); // Gravity is 1g
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
    uint8_t write_buffer[2] = {MPU6050_CONFIG_REG, 0x05};
    i2c_master_write_to_device(I2C_NUM_0, MPU6050_ADDR, write_buffer, sizeof(write_buffer), pdMS_TO_TICKS(1000));
}