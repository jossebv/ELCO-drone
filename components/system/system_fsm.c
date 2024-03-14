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

#include "esp_timer.h"

#include "system.h"
#include "sensors.h"
#include "controller.h"
#include "motors.h"

/* DEFINES */

#define CALIBRATION_TIME_US 5000000 // 5 seconds
#define CALIBRATION_THRESHOLD 0.5

/* TYPEDEFS */
typedef struct fsm_drone_t
{
    fsm_t fsm;
    uint64_t next;
    gyro_vector_t last_gyros;
    acc_vector_t last_acc;
} fsm_drone_t;

typedef enum system_fsm_states
{
    CALIBRATING = 0,
    FLYING,
    LANDING,
} system_fsm_states_t;

/* FUNCTIONS DECLARATIONS */
void system_fsm_init(fsm_t *fsm);

int is_drone_still_and_under_time(fsm_t *fsm);
int is_drone_moving_and_under_time(fsm_t *fsm);
int is_calibration_finished(fsm_t *fsm);
int is_battery_above_threshold_and_controller_connected(fsm_t *fsm);
int is_battery_below_threshold_or_controller_disconnected(fsm_t *fsm);

void do_update_calibration_progress(fsm_t *fsm);
void do_reset_calibration_progress(fsm_t *fsm);
void do_update_drone_motors(fsm_t *fsm);
void do_start_landing(fsm_t *fsm);

/* VARIABLES */

/* PUBLIC FUNCTIONS */

/**
 * @brief Create the system finite state machine
 *
 * @return fsm_t* The system finite state machine
 */
fsm_t *system_fsm_create()
{

    fsm_t *fsm = (fsm_t *)malloc(sizeof(fsm_drone_t));
    system_fsm_init(fsm);
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
void system_fsm_init(fsm_t *fsm)
{
    fsm_drone_t *fsm_drone = (fsm_drone_t *)fsm;
    static fsm_trans_t system_fsm_tt[] = {
        {CALIBRATING, is_drone_still_and_under_time, CALIBRATING, do_update_calibration_progress},
        {CALIBRATING, is_drone_moving_and_under_time, CALIBRATING, do_reset_calibration_progress},
        {CALIBRATING, is_calibration_finished, FLYING, NULL},
        {FLYING, is_battery_above_threshold_and_controller_connected, FLYING, do_update_drone_motors},
        {FLYING, is_battery_below_threshold_or_controller_disconnected, LANDING, do_start_landing},
        {-1, NULL, -1, NULL}};

    fsm_init(fsm, system_fsm_tt);
    fsm_drone->next = esp_timer_get_time() + CALIBRATION_TIME_US;
    //  fsm_drone->last_acc = get_accelerometer_data();
    //  fsm_drone->last_gyros = get_gyroscope_data();
}

/* PRIVATE FUNCTIONS */
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
 *
 * @return true
 * @return false
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
 * @brief Checks if the battery is above threshold and the controller is connected
 *
 * @return true
 * @return false
 */
int is_battery_above_threshold_and_controller_connected(fsm_t *fsm)
{
    // TODO: Implement the logic to check if the battery is above threshold and the controller is connected
    // printf("Checking if the battery is above threshold and the controller is connected\n");
    return 1; // At the moment we return true
}

int is_battery_below_threshold_or_controller_disconnected(fsm_t *fsm)
{
    return !is_battery_above_threshold_and_controller_connected(fsm);
}

/**
 * @brief Update the calibration progress
 *
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
 * @brief Update the drone status (sensors, motors, etc)
 *
 */
void do_update_drone_motors(fsm_t *fsm)
{
    // command_t command;

    drone_data_t sensors_data = sensors_update_drone_data();
    // controller_get_command(&command);

    // motors_update(command, sensors_data);
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
