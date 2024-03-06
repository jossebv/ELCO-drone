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

#define WIFI_RX_TX_PACKET_SIZE (64)

/* Structure used for in/out data via USB */
typedef struct
{
    uint8_t size;
    uint8_t data[WIFI_RX_TX_PACKET_SIZE];
} UDPPacket;

void wifi_init();
bool wifiGetDataBlocking(UDPPacket *in);
bool wifiSendData(UDPPacket *out);

#endif // WIFI_H