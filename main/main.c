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
    system_init();
    while (1)
    {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
