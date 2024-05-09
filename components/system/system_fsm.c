/**
 * @file system_fsm.c
 * @author Jose Manuel Bravo Pacheco
 * @brief Logic for the system finite state machine
 * @version 0.1
 * @date 2024-03-04
 *
 * @copyright Copyright (c) 2024
 *
 */

/* INCLUDES */
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "esp_timer.h"

#include "system.h"
#include "sensors.h"
#include "controller.h"
#include "motors.h"
#include "wifi.h"
#include "led.h"
#include "adc.h"

/* DEFINES */

#define CALIBRATION_TIME_US 10000000 /**< Time for the calibration in microseconds */
#define CALIBRATION_THRESHOLD 0.5    /**< Threshold for the calibration. The IMU variations will not reset the calibration if within this interval */

/* TYPEDEFS */
/**
 * @brief FSM structure for the system
 *
 */
typedef struct fsm_drone_t
{
    fsm_t fsm;                /**< Finite state machine */
    fsm_t *green_led_fsm;     /**< Pointer to green led finite state machine */
    fsm_t *blue_led_fsm;      /**< Pointer to blue led finite state machine */
    fsm_t *red_led_fsm;       /**< Pointer to red led finite state machine */
    uint64_t next;            /**< Next time to update the calibration */
    gyro_vector_t last_gyros; /**< Last gyroscope data */
    acc_vector_t last_acc;    /**< Last accelerometer data */
    uint32_t battery;         /**< Battery level */
} fsm_drone_t;

/**
 * @brief Enum with the states of the system finite state machine
 *
 */
typedef enum system_fsm_states
{
    CALIBRATING = 0,
    WAITING_CONTROLLER,
    FLYING,
    LANDING,
} system_fsm_states_t;

/* FUNCTIONS DECLARATIONS */
void system_fsm_init(fsm_t *fsm, fsm_t *green_led_fsm, fsm_t *blue_led_fsm, fsm_t *red_led_fsm);

int is_drone_still_and_under_time(fsm_t *fsm);
int is_drone_moving_and_under_time(fsm_t *fsm);
int is_calibration_finished(fsm_t *fsm);
int is_controller_connected(fsm_t *fsm);
int is_battery_below_threshold(fsm_t *fsm);
int is_battery_above_threshold_and_controller_connected(fsm_t *fsm);
int is_battery_below_threshold_or_controller_disconnected(fsm_t *fsm);

void do_update_calibration_progress(fsm_t *fsm);
void do_reset_calibration_progress(fsm_t *fsm);
void do_finish_calibration(fsm_t *fsm);
void do_controller_connected(fsm_t *fsm);
void do_update_drone_motors(fsm_t *fsm);
void do_inform_battery_below_threshold(fsm_t *fsm);
void do_start_landing(fsm_t *fsm);

/* VARIABLES */

/* PUBLIC FUNCTIONS */

/**
 * @brief Create the system finite state machine
 *
 * @return fsm_t* The system finite state machine
 */
fsm_t *system_fsm_create(fsm_t *green_led_fsm, fsm_t *blue_led_fsm, fsm_t *red_led_fsm)
{

    fsm_t *fsm = (fsm_t *)malloc(sizeof(fsm_drone_t));
    system_fsm_init(fsm, green_led_fsm, blue_led_fsm, red_led_fsm);
    return fsm;
}

/**
 * @brief Destroy the system finite state machine
 *
 * @param fsm The system finite state machine
 */
void system_fsm_destroy(fsm_t *fsm)
{
    free(fsm);
}

/**
 * @brief Initializes the system fsm
 *
 */
void system_fsm_init(fsm_t *fsm, fsm_t *green_led_fsm, fsm_t *blue_led_fsm, fsm_t *red_led_fsm)
{
    fsm_drone_t *fsm_drone = (fsm_drone_t *)fsm;
    static fsm_trans_t system_fsm_tt[] = {
        {CALIBRATING, is_drone_still_and_under_time, CALIBRATING, do_update_calibration_progress},
        {CALIBRATING, is_drone_moving_and_under_time, CALIBRATING, do_reset_calibration_progress},
        {CALIBRATING, is_calibration_finished, WAITING_CONTROLLER, do_finish_calibration},
        {WAITING_CONTROLLER, is_controller_connected, FLYING, do_controller_connected},
        {FLYING, is_battery_above_threshold_and_controller_connected, FLYING, do_update_drone_motors},
        {FLYING, is_battery_below_threshold, FLYING, do_inform_battery_below_threshold},
        {FLYING, is_battery_below_threshold_or_controller_disconnected, LANDING, do_start_landing},
        {-1, NULL, -1, NULL}};

    fsm_init(fsm, system_fsm_tt);
    fsm_drone->next = esp_timer_get_time() + CALIBRATION_TIME_US;
    fsm_drone->green_led_fsm = green_led_fsm;
    fsm_drone->blue_led_fsm = blue_led_fsm;
    fsm_drone->red_led_fsm = red_led_fsm;
    led_fsm_set_off(fsm_drone->red_led_fsm);
    //  fsm_drone->last_acc = get_accelerometer_data();
    //  fsm_drone->last_gyros = get_gyroscope_data();
}

/* PRIVATE FUNCTIONS */
/**
 * @brief Checks if the drone has not been moved during the calibration
 *
 * @param fsm Pointer to the finite state machine
 * @return int true if the drone has not been moved, false otherwise
 */
int is_drone_still(fsm_t *fsm)
{
    fsm_drone_t *fsm_drone = (fsm_drone_t *)fsm;
    gyro_vector_t gyros = fsm_drone->last_gyros;
    acc_vector_t acc = fsm_drone->last_acc;

    return (fabs(gyros.pitch) <= CALIBRATION_THRESHOLD &&
            fabs(gyros.roll) <= CALIBRATION_THRESHOLD &&
            fabs(gyros.yaw) <= CALIBRATION_THRESHOLD &&
            fabs(acc.x) <= CALIBRATION_THRESHOLD &&
            fabs(acc.y) <= CALIBRATION_THRESHOLD &&
            fabs((acc.z - 1)) <= CALIBRATION_THRESHOLD);
}

/**
 * @brief Checks if the drone is still during the calibration
 * @param fsm Pointer to the finite state machine
 * @return true if the drone is still, false otherwise
 */
int is_drone_still_and_under_time(fsm_t *fsm)
{
    fsm_drone_t *fsm_drone = (fsm_drone_t *)fsm;
    uint32_t now = esp_timer_get_time();

    return (is_drone_still(fsm) && now < fsm_drone->next);
}

/**
 * @brief Checks if the drone is moving during the calibration
 *
 * @param fsm
 * @return int
 */
int is_drone_moving_and_under_time(fsm_t *fsm)
{
    fsm_drone_t *fsm_drone = (fsm_drone_t *)fsm;
    uint32_t now = esp_timer_get_time();

    return (!is_drone_still(fsm) && now < fsm_drone->next);
}

/**
 * @brief Checks if the calibration is finished
 *
 * @return true
 * @return false
 */
int is_calibration_finished(fsm_t *fsm)
{
    fsm_drone_t *fsm_drone = (fsm_drone_t *)fsm;
    uint64_t now = esp_timer_get_time();
    return fsm_drone->next <= now;
}

/**
 * @brief Checks if the controller is connected
 *
 * @param fsm
 * @return int
 */
int is_controller_connected(fsm_t *fsm)
{
    return controller_is_connected();
}

/**
 * @brief Is the battery below threshold
 *
 */
int is_battery_below_threshold(fsm_t *fsm)
{
    fsm_drone_t *fsm_drone = (fsm_drone_t *)fsm;
    return fsm_drone->battery < 2625;
}

/**
 * @brief Checks if the battery is above threshold and the controller is connected
 *
 * @param fsm Pointer to the finite state machine
 * @return true if the battery is above threshold and the controller is connected, false otherwise
 */
int is_battery_above_threshold_and_controller_connected(fsm_t *fsm)
{
    // TODO: Implement the logic to check if the battery is above threshold and the controller is connected
    // printf("Checking if the battery is above threshold and the controller is connected\n");
    return 1; // At the moment we return true
}

/**
 * @brief Checks if the battery is below threshold or the controller is disconnected
 *
 * @param fsm Pointer to the finite state machine
 * @return int true if the battery is below threshold or the controller is disconnected, false otherwise
 */
int is_battery_below_threshold_or_controller_disconnected(fsm_t *fsm)
{
    return !is_battery_above_threshold_and_controller_connected(fsm);
}

/**
 * @brief Update the calibration progress
 *
 * @param fsm Pointer to the finite state machine
 */
void do_update_calibration_progress(fsm_t *fsm)
{
    fsm_drone_t *fsm_drone = (fsm_drone_t *)fsm;
    sensors_calibrate_imu(fsm_drone->last_gyros, fsm_drone->last_acc);

    fsm_drone->last_acc = get_accelerometer_data();
    fsm_drone->last_gyros = get_gyroscope_data();
}

/**
 * @brief Resets the calibration progress
 *
 * @param fsm
 */
void do_reset_calibration_progress(fsm_t *fsm)
{
    fsm_drone_t *fsm_drone = (fsm_drone_t *)fsm;
    sensors_calibrate_imu(fsm_drone->last_gyros, fsm_drone->last_acc);
    fsm_drone->next = esp_timer_get_time() + CALIBRATION_TIME_US;

    fsm_drone->last_acc = get_accelerometer_data();
    fsm_drone->last_gyros = get_gyroscope_data();

    printf("Resetting calibration progress\n");
}

/**
 * @brief Terminates the calibration process turning the green led on
 *
 * @param fsm
 */
void do_finish_calibration(fsm_t *fsm)
{
    fsm_drone_t *fsm_drone = (fsm_drone_t *)fsm;
    led_fsm_set_on(fsm_drone->green_led_fsm);
    printf("Calibration finished\n");
}

/**
 * @brief Checks that the controller is connected turning the blue led on
 *
 * @param fsm
 */
void do_controller_connected(fsm_t *fsm)
{
    fsm_drone_t *fsm_drone = (fsm_drone_t *)fsm;
    led_fsm_set_on(fsm_drone->blue_led_fsm);
    motors_reset();
    printf("Controller connected\n");
}

/**
 * @brief Update the drone status (sensors, motors, etc)
 *
 */
void do_update_drone_motors(fsm_t *fsm)
{
    drone_data_t sensors_data = sensors_update_drone_data();

    command_t command;
    // uint64_t t1 = esp_timer_get_time();
    controller_get_command(&command);
    // uint64_t t2 = esp_timer_get_time();
    // printf("Sensors update time: %.2f\n", (t2 - t1) / 1000.0);
    motors_update(command, sensors_data);

    // Send battery data to the monitor
    // static char packet[] = {0x40, 0x00, 0x00, 0x00, 0x00};
    fsm_drone_t *fsm_drone = (fsm_drone_t *)fsm;
    fsm_drone->battery = adc_read_voltage();
    // memcpy(&packet[1], &battery, sizeof(battery));
    // wifi_send_data(packet);
}

/**
 * @brief Inform that the battery is below threshold
 *
 */
void do_inform_battery_below_threshold(fsm_t *fsm)
{
    fsm_drone_t *fsm_drone = (fsm_drone_t *)fsm;
    led_fsm_set_on(fsm_drone->red_led_fsm);
    printf("Battery below threshold\n");

    do_update_drone_motors(fsm);
}

/**
 * @brief Start the landing process
 *
 */
void do_start_landing(fsm_t *fsm)
{
    // TODO: Implement the logic to start landing
    printf("Starting landing\n");
}
