/**
 * @file controller.c
 * @author Jose Manuel Bravo
 * @brief Functionality of the remote controller
 * @version 0.1
 * @date 2024-03-06
 *
 * @copyright Copyright (c) 2024
 *
 */

#include <stdio.h>

#include "controller.h"
#include "wifi.h"

#define DEBUG_CONTROLLER 1

/**
 * @brief Decode the command from the packet
 *
 * @param packet
 * @param command
 */
void decode_command(UDPPacket *packet, command_t *command)
{
    union
    {
        long y;
        float z;
    } data;
    command->thrust = (uint8_t)packet->data[14] * 1000 / 204;
    long yaw = (float)(((long)packet->data[12] << 24) | ((long)packet->data[11] << 16) | ((long)packet->data[10] << 8) | (uint8_t)packet->data[9]);
    data.y = yaw;
    command->yaw_speed = data.z;
    long pitch = (float)(((long)packet->data[8] << 24) | ((long)packet->data[7] << 16) | ((long)packet->data[6] << 8) | (uint8_t)packet->data[5]);
    data.y = pitch;
    command->pitch = data.z;
    long roll = (float)(((long)packet->data[4] << 24) | ((long)packet->data[3] << 16) | ((long)packet->data[2] << 8) | (uint8_t)packet->data[1]);
    data.y = roll;
    command->roll = data.z;
}

/**
 * @brief Get the command from the remote controller
 *
 * @param command
 */
void controller_get_command(command_t *command)
{
    UDPPacket packet;
    wifiGetDataBlocking(&packet);
    decode_command(&packet, command);

#if DEBUG_CONTROLLER
    printf("Command received: \n");
    printf("Thrust: %d\n", command->thrust);
    printf("Yaw: %f\n", command->yaw_speed);
    printf("Pitch: %f\n", command->pitch);
    printf("Roll: %f\n", command->roll);
#endif
}