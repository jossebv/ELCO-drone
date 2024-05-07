/**
 * @file comms.c
 * @author José Manuel Bravo
 * @brief Handle all the communications between the drone and the ground station.
 * @version 0.1
 * @date 2024-04-25
 *
 * @copyright Copyright (c) 2024
 *
 */

/* INCLUDES */
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "wifi.h"
#include "comms.h"
#include "motors.h"

#define PID_UPDATE_HEADER 0x51

void process_instruction(UDPPacket *instruction)
{
    switch (instruction->data[1])
    {
    case PID_UPDATE_HEADER:
        printf("PID_UPDATE_HEADER\n");

        uint8_t pid_number = (uint8_t)instruction->data[2];
        float kp, ki, kd;

        memcpy(&kp, &instruction->data[3], sizeof(kp));
        memcpy(&ki, &instruction->data[7], sizeof(ki));
        memcpy(&kd, &instruction->data[11], sizeof(kd));

        printf("Updating pid_num: %d, kp: %.5f, ki: %.5f, kd: %.5f\n", pid_number, kp, ki, kd);
        fflush(stdout);

        // Update PID
        if (motors_update_pid_constants(pid_number, kp, ki, kd))
        {
            printf("PID updated successfully\n");
        }
        else
        {
            printf("PID update failed\n");
        }

        break;

    default:
        break;
    }
}

void comms_task(void *pvParameters)
{
    UDPPacket instruction;
    while (1)
    {
        if (wifi_get_instruction_blocking(&instruction))
        {
            process_instruction(&instruction);
        }
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

void comms_init()
{
    xTaskCreate(&comms_task, "comms_task", 4096, NULL, 3, NULL);
}