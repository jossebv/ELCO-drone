/**
 * @file main.c
 * @author Jose Manuel Bravo
 * @brief Main file for the drone
 * @version 0.1
 * @date 2024-02-29
 *
 * @copyright Copyright (c) 2024
 *
 */

/* INCLUDES */
#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "main.h"
#include "system.h"

void app_main(void)
{
    xTaskCreate(system_task, "system_task", 4096, NULL, 10, NULL);
}
