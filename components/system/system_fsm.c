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

#include "system.h"

/* DEFINES */

/* TYPEDEFS */
typedef enum system_fsm_states
{
    IDLE = 0,
    CALIBRATING,
    FLYING,
    LANDING,
} system_fsm_states_t;

/* FUNCTIONS DECLARATIONS */
int is_drone_still();
int is_calibration_finished();
int is_battery_above_threshold_and_controller_connected();
int is_battery_below_threshold_or_controller_disconnected();

void do_update_calibration_progress();
void do_finish_calibration();
void do_update_drone();
void do_start_landing();

/* VARIABLES */

/* PUBLIC FUNCTIONS */
fsm_t *system_fsm_create()
{
    fsm_trans_t system_fsm_tt[] = {
        {CALIBRATING, is_drone_still, CALIBRATING, do_update_calibration_progress},
        {CALIBRATING, is_calibration_finished, FLYING, do_finish_calibration},
        {FLYING, is_battery_above_threshold_and_controller_connected, FLYING, do_update_drone},
        {FLYING, is_battery_below_threshold_or_controller_disconnected, LANDING, do_start_landing},
        {-1, NULL, -1, NULL}};

    fsm_t *fsm_drone = fsm_new(system_fsm_tt);
    return fsm_drone;
}

/* PRIVATE FUNCTIONS */
/**
 * @brief Checks if the drone is still
 *
 * @return true
 * @return false
 */
int is_drone_still()
{
    // TODO: Implement the logic to check if the drone is still
    printf("Checking if the drone is still\n");
    return 1;
}

/**
 * @brief Checks if the calibration is finished
 *
 * @return true
 * @return false
 */
int is_calibration_finished()
{
    // TODO: Implement the logic to check if the calibration is finished
    printf("Checking if the calibration is finished\n");
    return 1;
}

/**
 * @brief Checks if the battery is above threshold and the controller is connected
 *
 * @return true
 * @return false
 */
int is_battery_above_threshold_and_controller_connected()
{
    // TODO: Implement the logic to check if the battery is above threshold and the controller is connected
    printf("Checking if the battery is above threshold and the controller is connected\n");
    return 1;
}

int is_battery_below_threshold_or_controller_disconnected()
{
    return !is_battery_above_threshold_and_controller_connected();
}

/**
 * @brief Update the calibration progress
 *
 */
void do_update_calibration_progress()
{
    // TODO: Implement the logic to update the calibration progress
    printf("Updating calibration progress\n");
}

/**
 * @brief Finish the calibration
 *
 */
void do_finish_calibration()
{
    // TODO: Implement the logic to finish the calibration
    printf("Finishing calibration\n");
}

/**
 * @brief Update the drone status (sensors, motors, etc)
 *
 */
void do_update_drone()
{
    // TODO: Implement the logic to update the drone
    printf("Updating drone\n");
}

/**
 * @brief Start the landing process
 *
 */
void do_start_landing()
{
    // TODO: Implement the logic to start landing
    printf("Starting landing\n");
}
