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
#include <stdbool.h>
#include <stdio.h>

#include "esp_timer.h"

#include "system.h"
#include "sensors.h"

/* DEFINES */

/* TYPEDEFS */
typedef struct fsm_drone_t
{
    fsm_t *fsm;
    uint64_t next;
    gyro_vector_t last_gyros;
    acc_vector_t last_acc;
} fsm_drone_t;

typedef enum system_fsm_states
{
    IDLE = 0,
    CALIBRATING,
    FLYING,
    LANDING,
} system_fsm_states_t;

/* FUNCTIONS DECLARATIONS */
int is_drone_still_and_under_time(fsm_t *fsm);
int is_calibration_finished(fsm_t *fsm);
int is_battery_above_threshold_and_controller_connected(fsm_t *fsm);
int is_battery_below_threshold_or_controller_disconnected(fsm_t *fsm);

void do_update_calibration_progress(fsm_t *fsm);
void do_finish_calibration(fsm_t *fsm);
void do_update_drone(fsm_t *fsm);
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
    fsm_trans_t system_fsm_tt[] = {
        {CALIBRATING, is_drone_still_and_under_time, CALIBRATING, do_update_calibration_progress},
        {CALIBRATING, is_calibration_finished, FLYING, do_finish_calibration},
        {FLYING, is_battery_above_threshold_and_controller_connected, FLYING, do_update_drone},
        {FLYING, is_battery_below_threshold_or_controller_disconnected, LANDING, do_start_landing},
        {-1, NULL, -1, NULL}};

    fsm_drone_t *fsm_drone = (fsm_drone_t *)malloc(sizeof(fsm_drone_t));
    fsm_drone->fsm = fsm_new(system_fsm_tt);
    return (fsm_t *)fsm_drone;
}

/* PRIVATE FUNCTIONS */
/**
 * @brief Checks if the drone is still
 *
 * @return true
 * @return false
 */
int is_drone_still_and_under_time(fsm_t *fsm)
{
    fsm_drone_t *fsm_drone = (fsm_drone_t *)fsm;

    double threshold = 0.2;
    uint64_t now = esp_timer_get_time();
    gyro_vector_t gyros = fsm_drone->last_gyros;
    acc_vector_t acc = fsm_drone->last_acc;

    return (gyros.pitch <= threshold && gyros.roll <= threshold && gyros.yaw <= threshold && acc.x <= threshold && acc.y <= threshold && acc.z <= threshold && now < fsm_drone->next);
}

int is_drone_moving_and_under_time(fsm_t *fsm)
{
    fsm_drone_t *fsm_drone = (fsm_drone_t *)fsm;

    double threshold = 0.2;
    uint64_t now = esp_timer_get_time();
    gyro_vector_t gyros = fsm_drone->last_gyros;
    acc_vector_t acc = fsm_drone->last_acc;

    return ((gyros.pitch > threshold || gyros.roll > threshold || gyros.yaw > threshold || acc.x > threshold || acc.y > threshold || acc.z > threshold) && now < fsm_drone->next);
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
    printf("Checking if the battery is above threshold and the controller is connected\n");
    return 1;
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
    // TODO: Implement the logic to update the calibration progress
    printf("Updating calibration progress\n");
}

/**
 * @brief Finish the calibration
 *
 */
void do_finish_calibration(fsm_t *fsm)
{
    // TODO: Implement the logic to finish the calibration
    printf("Finishing calibration\n");
}

/**
 * @brief Update the drone status (sensors, motors, etc)
 *
 */
void do_update_drone_motors(fsm_t *fsm)
{
    // TODO: Implement the logic to update the drone

        printf("Updating drone\n");
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
