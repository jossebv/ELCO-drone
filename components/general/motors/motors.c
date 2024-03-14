/**
 * @file motors.c
 * @author Jose Manuel Bravo
 * @brief Contains the functionality for controlling the drone motors.
 * @version 0.1
 * @date 2024-02-29
 *
 * @copyright Copyright (c) 2024
 *
 */

/* INCLUDES */
#include <stdbool.h>
#include <stdint.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_timer.h"
#include "driver/gpio.h"

#include "motors.h"
#include "pid.h"
#include "controller.h"
#include "sensors.h"

/* DEFINES */
#define PITCH_KP 1
#define PITCH_KI 0
#define PITCH_KD 0

#define ROLL_KP 1
#define ROLL_KI 0
#define ROLL_KD 0

#define YAW_KP 1
#define YAW_KI 0
#define YAW_KD 0

#define MOTOR_MIN 1000
#define MOTOR_MAX 2000
#define THROTTLE_MIN 1000
#define THROTTLE_MAX 1700

#define MOTOR1_PIN GPIO_NUM_1
#define MOTOR2_PIN GPIO_NUM_2
#define MOTOR3_PIN GPIO_NUM_3
#define MOTOR4_PIN GPIO_NUM_4

/* VARIABLES */
static bool is_init = false;
static pid_data_t *pid_pitch;
static pid_data_t *pid_roll;
static pid_data_t *pid_yaw;

/* FUNCTIONS DECLARATIONS */

/* PUBLIC FUNCTIONS */
/**
 * @brief Inits all the motors
 *
 */
void motors_init()
{
    if (is_init)
    {
        return;
    }

    // Initialize the motors pins
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = (1ULL << MOTOR1_PIN) | (1ULL << MOTOR2_PIN) | (1ULL << MOTOR3_PIN) | (1ULL << MOTOR4_PIN);
    io_conf.pull_down_en = 0;
    io_conf.pull_up_en = 0;
    gpio_config(&io_conf);

    // Initialize the PID controllers
    pid_pitch = pid_create(PITCH_KP, PITCH_KI, PITCH_KD);
    pid_roll = pid_create(ROLL_KP, ROLL_KI, ROLL_KD);
    pid_yaw = pid_create(YAW_KP, YAW_KI, YAW_KD);

    is_init = true;
}

/**
 * @brief Deinits all the motors
 *
 */
void motors_deinit()
{
    if (!is_init)
    {
        return;
    }

    pid_destroy(pid_pitch);
    pid_destroy(pid_roll);
    pid_destroy(pid_yaw);

    is_init = false;
}

/**
 * @brief Normalize the motor overall value to fit the range
 *
 * @param motor1_us
 * @param motor2_us
 * @param motor3_us
 * @param motor4_us
 */
void normalize_motor_values(uint16_t *motor1_us, uint16_t *motor2_us, uint16_t *motor3_us, uint16_t *motor4_us)
{
    if (*motor1_us < MOTOR_MIN)
    {
        *motor1_us = MOTOR_MIN;
    }
    if (*motor1_us > MOTOR_MAX)
    {
        *motor1_us = MOTOR_MAX;
    }

    if (*motor2_us < MOTOR_MIN)
    {
        *motor2_us = MOTOR_MIN;
    }
    if (*motor2_us > MOTOR_MAX)
    {
        *motor2_us = MOTOR_MAX;
    }

    if (*motor3_us < MOTOR_MIN)
    {
        *motor3_us = MOTOR_MIN;
    }
    if (*motor3_us > MOTOR_MAX)
    {
        *motor3_us = MOTOR_MAX;
    }

    if (*motor4_us < MOTOR_MIN)
    {
        *motor4_us = MOTOR_MIN;
    }
    if (*motor4_us > MOTOR_MAX)
    {
        *motor4_us = MOTOR_MAX;
    }
}

/**
 * @brief Normalize the thrust value to fit the range
 *
 * @param thrust
 */
void normalize_thrust_value(uint16_t *thrust)
{
    if (*thrust < THROTTLE_MIN)
    {
        *thrust = THROTTLE_MIN;
    }
    if (*thrust > THROTTLE_MAX)
    {
        *thrust = THROTTLE_MAX;
    }
}

/**
 * @brief Send the values to the ESC
 *
 * @param motor1 Value for motor 1
 * @param motor2 Value for motor 2
 * @param motor3 Value for motor 3
 * @param motor4 Value for motor 4
 */
void motors_send_values_to_esc_blocking(uint16_t motor1_us, uint16_t motor2_us, uint16_t motor3_us, uint16_t motor4_us)
{
    gpio_set_level(MOTOR1_PIN, 1);
    gpio_set_level(MOTOR2_PIN, 1);
    gpio_set_level(MOTOR3_PIN, 1);
    gpio_set_level(MOTOR4_PIN, 1);

    uint32_t now = esp_timer_get_time();
    uint32_t end1 = now + motor1_us;
    uint32_t end2 = now + motor2_us;
    uint32_t end3 = now + motor3_us;
    uint32_t end4 = now + motor4_us;

    while (now < end1 || now < end2 || now < end3 || now < end4)
    {
        now = esp_timer_get_time();
        if (now < end1)
        {
            gpio_set_level(MOTOR1_PIN, 0);
        }
        if (now < end2)
        {
            gpio_set_level(MOTOR2_PIN, 0);
        }
        if (now < end3)
        {
            gpio_set_level(MOTOR3_PIN, 0);
        }
        if (now < end4)
        {
            gpio_set_level(MOTOR4_PIN, 0);
        }
    }
}

/**
 * @brief Update the motors
 *
 * @param command Command to be executed
 * @param drone_data Data from the drone
 *
 *
 *  MOTORS CONFIGURATION
 *
 *         1   2
 *          \ /
 *           X
 *          / \
 *         4   3
 *
 */
void motors_update(command_t command, drone_data_t drone_data)
{
    double pid_pitch_value = pid_update(pid_pitch, command.pitch - drone_data.pitch);
    double pid_roll_value = pid_update(pid_roll, command.roll - drone_data.roll);
    double pid_yaw_value = pid_update(pid_yaw, command.yaw - drone_data.yaw_speed);

    normalize_thrust_value(&command.thrust);

    uint16_t motor1 = command.thrust + pid_pitch_value + pid_roll_value + pid_yaw_value;
    uint16_t motor2 = command.thrust + pid_pitch_value - pid_roll_value - pid_yaw_value;
    uint16_t motor3 = command.thrust - pid_pitch_value - pid_roll_value + pid_yaw_value;
    uint16_t motor4 = command.thrust - pid_pitch_value + pid_roll_value - pid_yaw_value;

    normalize_motor_values(&motor1, &motor2, &motor3, &motor4);
    motors_send_values_to_esc_blocking(motor1, motor2, motor3, motor4);
}