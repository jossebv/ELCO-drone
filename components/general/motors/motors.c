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
#include "driver/ledc.h"
#include "esp_log.h"

#include "main.h"
#include "motors.h"
#include "pid.h"
#include "controller.h"
#include "sensors.h"

/* DEFINES */
#define PITCH_KP 1 // Proportional constant for the pith PID controller
#define PITCH_KI 0 // Integral constant for the pith PID controller
#define PITCH_KD 0 // Derivative constant for the pith PID controller

#define ROLL_KP PITCH_KP // Due to the symmetry of the drone
#define ROLL_KI PITCH_KI // Due to the symmetry of the drone
#define ROLL_KD PITCH_KD // Due to the symmetry of the drone

#define YAW_KP 1 // Proportional constant for the yaw PID controller
#define YAW_KI 0 // Integral constant for the yaw PID controller
#define YAW_KD 0 // Derivative constant for the yaw PID controller

#define MOTOR_MIN_US 1000 // Minimum value for the motors
#define MOTOR_MAX_US 2000 // Maximum value for the motors
#define THROTTLE_MIN 1000 // Minimum value for the throttle
#define THROTTLE_MAX 1700 // Maximum value for the throttle. Should be lower than MOTOR_MAX

#define MOTOR1_PIN GPIO_NUM_1 // Pin for motor 1
#define MOTOR2_PIN GPIO_NUM_2 // Pin for motor 2
#define MOTOR3_PIN GPIO_NUM_3 // Pin for motor 3
#define MOTOR4_PIN GPIO_NUM_4 // Pin for motor 4

/* VARIABLES */
static const char *TAG = "motors";
static const uint8_t MOTOR_PINS[4] = {MOTOR1_PIN, MOTOR2_PIN, MOTOR3_PIN, MOTOR4_PIN};
static const uint16_t MOTOR_MIN_DUTY = (MOTOR_MIN_US * 65535 / 5000);
static const uint16_t MOTOR_MAX_DUTY = (MOTOR_MAX_US * 65535 / 5000);

static bool is_init = false;
static pid_data_t *pid_pitch;
static pid_data_t *pid_roll;
static pid_data_t *pid_yaw;

/* FUNCTIONS DECLARATIONS */

void _motors_ledc_init()
{
    ledc_timer_config_t ledc_timer = {
        .duty_resolution = LEDC_TIMER_16_BIT, // resolution of PWM duty
        .freq_hz = DRONE_UPDATE_FREQ,         // frequency of PWM signal
        .speed_mode = LEDC_LOW_SPEED_MODE,    // timer mode
        .timer_num = LEDC_TIMER_0,            // timer index
        .clk_cfg = LEDC_AUTO_CLK,             // Auto select the source clock
    };

    ledc_timer_config(&ledc_timer);

    ledc_channel_config_t ledc_channel = {
        .channel = LEDC_CHANNEL_0,
        .duty = MOTOR_MIN_DUTY,
        .gpio_num = MOTOR1_PIN,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .timer_sel = LEDC_TIMER_0,
    };

    for (int i = 0; i < 4; i++)
    {
        ledc_channel.gpio_num = MOTOR_PINS[i];
        ledc_channel.channel = LEDC_CHANNEL_0 + i;
        ledc_channel_config(&ledc_channel);
    }

    ESP_LOGI(TAG, "PWM initialized");
}

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

    // Initialize the LEDC (PWM signals)
    _motors_ledc_init();

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
 * @brief Normalize the thrust value to fit the range (THROTTLE_MIN, THROTTLE_MAX)
 *
 * @param thrust Throttle value, expected to be between 0 and 1000
 */
void normalize_thrust_value(uint16_t *thrust)
{
    *thrust = ((*thrust) * ((THROTTLE_MAX - THROTTLE_MIN) / 1000)) + THROTTLE_MIN;
}

/**
 * @brief Normalize the motor overall value to fit the range (MOTOR_MIN_DUTY, MOTOR_MAX_DUTY)
 *
 * @param motor_duties Duties for the motors
 */
void normalize_motor_duties(uint16_t *motor_duties)
{
    for (int i = 0; i < 4; i++)
    {
        if (motor_duties[i] < MOTOR_MIN_DUTY)
        {
            motor_duties[i] = MOTOR_MIN_DUTY;
        }
        if (motor_duties[i] > MOTOR_MAX_DUTY)
        {
            motor_duties[i] = MOTOR_MAX_DUTY;
        }
    }
}

/**
 * @brief Change the duties of the motors
 *
 * @param motor_duties Duties for the motors
 */
void motors_change_duties(uint16_t *motor_duties)
{
    for (int i = 0; i < 4; i++)
    {
        ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0 + i, motor_duties[i]);
        ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0 + i);
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
    double pid_yaw_value = pid_update(pid_yaw, command.yaw_speed - drone_data.yaw_speed);

    normalize_thrust_value(&command.thrust);

    // TODO: Check if the pid_yaw_value are correct respect to the motors configuration (It depends on the direction they move).
    uint16_t motor1_duty = command.thrust + pid_pitch_value + pid_roll_value + pid_yaw_value;
    uint16_t motor2_duty = command.thrust + pid_pitch_value - pid_roll_value - pid_yaw_value;
    uint16_t motor3_duty = command.thrust - pid_pitch_value - pid_roll_value + pid_yaw_value;
    uint16_t motor4_duty = command.thrust - pid_pitch_value + pid_roll_value - pid_yaw_value;

    uint16_t motors_duties[4] = {motor1_duty, motor2_duty, motor3_duty, motor4_duty};

    normalize_motor_duties(motors_duties);
    motors_change_duties(motors_duties);
}