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

#include "esp_log.h"

#include "wifi.h"
#include "comms.h"
#include "motors.h"
#include "sensors.h"

#define PID_UPDATE_HEADER 0x51 /**< Header for the PID update */
#define REQ_IMU_HEADER 0x82    /**< Header for the IMU request */

static char *TAG = "Comms";

static char header = 0x82;

/**
 * @brief Handle the IMU request from the ground station
 *
 */
void handle_imu_req()
{
    drone_data_t data = sensors_get_drone_data();
    static char packet[sizeof(header) + sizeof(data.pitch) + sizeof(data.roll) + sizeof(data.yaw_speed)];

    packet[0] = header;
    memcpy(packet + 1, &data.pitch, sizeof(data.pitch));
    memcpy(packet + 1 + sizeof(data.pitch), &data.roll, sizeof(data.roll));
    memcpy(packet + 1 + sizeof(data.pitch) + sizeof(data.roll), &data.yaw_speed, sizeof(data.yaw_speed));

    wifi_send_data(packet, sizeof(packet));
}

/**
 * @brief Process the instruction received from the ground station
 *
 * @param instruction
 */
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

    case REQ_IMU_HEADER:
        handle_imu_req();
        // ESP_LOGI(TAG, "Sending drone data");
    default:
        break;
    }
}

/**
 * @brief Task for the communications module
 *
 * @param pvParameters
 */
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

/**
 * @brief Initialize the communications module
 *
 */
void comms_init()
{
    xTaskCreate(&comms_task, "comms_task", 4096, NULL, 3, NULL);
}