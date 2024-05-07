/**
 * @file wifi.h
 * @author Jose Manuel Bravo
 * @brief Header file for the wifi driver
 * @version 0.1
 * @date 2024-03-05
 *
 * @copyright Copyright (c) 2024
 *
 */

#ifndef WIFI_H
#define WIFI_H

#include <stdint.h>
#include <stdbool.h>

#define WIFI_RX_TX_PACKET_SIZE (64)

/* Structure used for in/out data via USB */
typedef struct
{
    uint8_t size;
    uint8_t data[WIFI_RX_TX_PACKET_SIZE];
} UDPPacket;

void wifi_init();
bool wifiGetDataBlocking(UDPPacket *in);
bool wifi_get_instruction_blocking(UDPPacket *instruction);
bool wifi_send_data(char *data, uint8_t size);
int wifiIsControllerConnected();

#endif // WIFI_H