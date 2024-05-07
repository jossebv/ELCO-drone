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
#include <string.h>

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
#include "wifi.h"

/* DEFINES */
#define PITCH_KP 0 // 0.32  // Proportional constant for the pith PID controller
#define PITCH_KI 0 // 0.005 // Integral constant for the pith PID controller
#define PITCH_KD 0 // 0.5   // Derivative constant for the pith PID controller

#define ROLL_KP 0 // 0.3   // Due to the symmetry of the drone
#define ROLL_KI 0 // 0.001 // Due to the symmetry of the drone
#define ROLL_KD 0 // 0.5   // Due to the symmetry of the drone

#define YAW_KP 1 // Proportional constant for the yaw PID controller
#define YAW_KI 0 // Integral constant for the yaw PID controller
#define YAW_KD 0 // Derivative constant for the yaw PID controller

#define PWM_PERIOD_MS 3                  // Period of the PWM signal
#define PWM_FREQ_HZ 1000 / PWM_PERIOD_MS // Frequency of the PWM signal

#define MOTOR_MIN_US 1000 // Minimum value for the motors
#define MOTOR_MAX_US 2000 // Maximum value for the motors
#define THROTTLE_MAX 80   // Maximum value for the throttle. Should be lower than MOTOR_MAX

#define MOTOR1_PIN GPIO_NUM_18 // Pin for motor 1
#define MOTOR2_PIN GPIO_NUM_5  // Pin for motor 2
#define MOTOR3_PIN GPIO_NUM_17 // Pin for motor 3
#define MOTOR4_PIN GPIO_NUM_16 // Pin for motor 4

/* VARIABLES */
static const char *TAG = "motors";
static const int MOTOR_PINS[4] = {MOTOR1_PIN, MOTOR2_PIN, MOTOR3_PIN, MOTOR4_PIN};
static const uint16_t MOTOR_MIN_DUTY = (MOTOR_MIN_US * 65535 / (PWM_PERIOD_MS * 1000));
static const uint16_t MOTOR_MAX_DUTY = (MOTOR_MAX_US * 65535 / (PWM_PERIOD_MS * 1000));

static bool is_init = false;
static pid_data_t *pid_pitch;
static pid_data_t *pid_roll;
static pid_data_t *pid_yaw;

/* FUNCTIONS DECLARATIONS */

void _motors_ledc_init()
{
    ledc_timer_config_t ledc_timer = {
        .duty_resolution = LEDC_TIMER_16_BIT, // resolution of PWM duty
        .freq_hz = PWM_FREQ_HZ,               // frequency of PWM signal
        .speed_mode = LEDC_HIGH_SPEED_MODE,   // timer mode
        .timer_num = LEDC_TIMER_0,            // timer index
        .clk_cfg = LEDC_AUTO_CLK,             // Auto select the source clock
    };

    ledc_timer_config(&ledc_timer);

    ledc_channel_config_t ledc_channel = {
        .channel = LEDC_CHANNEL_0,
        .duty = MOTOR_MIN_DUTY,
        .gpio_num = MOTOR1_PIN,
        .speed_mode = LEDC_HIGH_SPEED_MODE,
        .timer_sel = LEDC_TIMER_0,
    };

    // ledc_channel_config(&ledc_channel);

    for (int i = 0; i < 4; i++)
    {
        ledc_channel.gpio_num = MOTOR_PINS[i];
        ledc_channel.channel = i;
        ledc_channel_config(&ledc_channel);
    }

    ESP_LOGI(TAG, "PWM initialized");
}

void motor_update_pid_constants()
{
}

/**
 * @brief Inits all the motors
 *
 */
void motors_init()
{
    ESP_LOGI(TAG, "Initializing motors");

    if (is_init)
    {
        return;
    }

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
 * @brief Update the PID constants for a specific PID controller
 *
 * @param pid_number PID controller number (1: pitch, 2: roll, 3: yaw)
 * @param kp Proportional constant
 * @param ki Integral constant
 * @param kd Derivative constant
 * @return true
 * @return false
 */
bool motors_update_pid_constants(uint8_t pid_number, float kp, float ki, float kd)
{
    switch (pid_number)
    {
    case 1:
        pid_update_constants(pid_pitch, kp, ki, kd);
        break;
    case 2:
        pid_update_constants(pid_roll, kp, ki, kd);
        break;
    case 3:
        pid_update_constants(pid_yaw, kp, ki, kd);
        break;
    default:
        return false;
        break;
    }
    return true;
}

/**
 * @brief Normalize the thrust to make it a percentage between 0 and THROTTLE_MAX
 *
 * @param thrust Throttle value, expected to be between 0 and 1000
 */
void normalize_thrust_value(uint16_t *thrust)
{
    *thrust = (*thrust * THROTTLE_MAX / 1000);
}

/**
 * @brief Normalize the motor overall value to fit the range (MOTOR_MIN_DUTY, MOTOR_MAX_DUTY)
 *
 * @param motor_duties Duties for the motors
 */
void normalize_motor_duties(double *motor_duties)
{
    for (int i = 0; i < 4; i++)
    {
        if (motor_duties[i] < 0)
        {
            motor_duties[i] = 0;
            printf("Motor speed negative!!\n");
        }
        if (motor_duties[i] > 100)
        {
            motor_duties[i] = 100;
            printf("Motor speed over 100%%!!\n");
        }
    }
}

/**
 * @brief Change the duties of the motors
 *
 * @param motor_duties Motor speed as a percentage
 */
void motors_update_duties(double *motor_speeds)
{
    for (int i = 0; i < 4; i++)
    {
        uint16_t motor_duty = (motor_speeds[i] * (MOTOR_MAX_DUTY - MOTOR_MIN_DUTY) / 100) + MOTOR_MIN_DUTY;
        ledc_set_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0 + i, motor_duty);
        ledc_update_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0 + i);
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
    command.pitch = 0;
    command.roll = 0;

    double pid_pitch_value = 0;
    double pid_roll_value = 0;
    double pid_yaw_value = 0;

    if (command.thrust > 10)
    {
        pid_pitch_value = pid_update(pid_pitch, command.pitch - drone_data.pitch);
        pid_roll_value = pid_update(pid_roll, command.roll - drone_data.roll);
        pid_yaw_value = 0; // pid_update(pid_yaw, command.yaw_speed - drone_data.yaw_speed);
        printf("PID values: %.2f, %.2f, %.2f\n", pid_pitch_value, pid_roll_value, pid_yaw_value);
        printf("PID constants: %.2f %.2f %.2f", pid_roll->kp, pid_roll->ki, pid_roll->kd);
    }
    else if (command.thrust < 5)
    {
        pid_reset(pid_pitch);
        pid_reset(pid_roll);
        pid_reset(pid_yaw);
    }

    normalize_thrust_value(&command.thrust);

    // TODO: Check if the pid_yaw_value are correct respect to the motors configuration (It depends on the direction they move).
    double motor1_speed = (double)(command.thrust) + pid_pitch_value + pid_roll_value + pid_yaw_value;
    double motor2_speed = (double)(command.thrust) + pid_pitch_value - pid_roll_value - pid_yaw_value;
    double motor3_speed = (double)(command.thrust) - pid_pitch_value - pid_roll_value + pid_yaw_value;
    double motor4_speed = (double)(command.thrust) - pid_pitch_value + pid_roll_value - pid_yaw_value;

    double motors_speeds[4] = {motor1_speed, motor2_speed, motor3_speed, motor4_speed};

    // uint32_t motor_debug = (uint32_t)motor1_speed; // Just for debugging purposes

    // static char packet[] = {0x40, 0x00, 0x00, 0x00, 0x00};
    // memcpy(&packet[1], &motor_debug, sizeof(motor_debug));
    // wifi_send_data(packet);

    normalize_motor_duties(motors_speeds);
    motors_update_duties(motors_speeds);
}

/**
 * @brief Reset the motors
 *
 */
void motors_reset()
{
    pid_reset(pid_pitch);
    pid_reset(pid_roll);
    pid_reset(pid_yaw);
}