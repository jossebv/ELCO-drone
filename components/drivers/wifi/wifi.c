/**
 * @file wifi.c
 * @author Jose Manuel Bravo
 * @brief Driver for the wifi module
 * @version 0.1
 * @date 2024-03-05
 *
 * @copyright Copyright (c) 2024
 *
 */

/* INCLUDES */
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <netdb.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_wifi.h"

/* DEFINES */
#define DEBUG_UPD 0

#ifndef MAC2STR
#define MAC2STR(a) (a)[0], (a)[1], (a)[2], (a)[3], (a)[4], (a)[5]
#define MACSTR "%02x:%02x:%02x:%02x:%02x:%02x"
#endif

#define CONFIG_WIFI_BASE_SSID "ESP32_DRONE"
#define CONFIG_WIFI_PASSWORD "12345678"
#define CONFIG_WIFI_CHANNEL 1
#define CONFIG_WIFI_MAX_STA_CONN 4
#define WIFI_MAX_STA_CONN CONFIG_WIFI_MAX_STA_CONN

#define UDP_SERVER_PORT 2390
#define UDP_SERVER_BUFFSIZE 128

#define UDP_RX_TASK_STACKSIZE 2048
#define UDP_RX_TASK_PRI 3
#define UDP_TX_TASK_STACKSIZE 2048
#define UDP_TX_TASK_PRI 3

/* VARIABLES */
static const char *TAG = "wifi";
static bool is_init = false;
static bool is_udp_init = false;
static bool is_udp_controller_connected = false;
static bool is_udp_console_connected = false;

esp_netif_t *ap_netif;

static struct sockaddr_in console_addr;

static char rx_buffer[UDP_SERVER_BUFFSIZE];
static char tx_buffer[UDP_SERVER_BUFFSIZE];
static struct sockaddr_in dest_addr;
static int sock;

static UDPPacket in_packet;
static UDPPacket out_packet;

static QueueHandle_t udp_data_rx;
static QueueHandle_t udp_data_tx;

/* PRIVATE FUNCTIONS */
/**
 * @brief Get the checksum of a given data
 *
 * @param data pointer to the data
 * @param len length of the data
 * @return uint8_t checksum
 */
static uint8_t calculate_cksum(void *data, size_t len)
{
    unsigned char *c = data;
    int i;
    unsigned char cksum = 0;

    for (i = 0; i < len; i++)
    {
        cksum += *(c++);
    }

    return cksum;
}

/**
 * @brief Returns if the controller is connected by udp channel
 *
 * @return true
 * @return false
 */
int wifiIsControllerConnected()
{
    return (int)is_udp_controller_connected;
}

/**
 * @brief Event handler for the wifi module
 *
 * @param arg
 * @param event_base
 * @param event_id
 * @param event_data
 */
static void wifi_event_handler(void *arg, esp_event_base_t event_base,
                               int32_t event_id, void *event_data)
{
    if (event_id == WIFI_EVENT_AP_STACONNECTED)
    {
        wifi_event_ap_staconnected_t *event = (wifi_event_ap_staconnected_t *)event_data;
        ESP_LOGI(TAG, "station " MACSTR " join, AID=%d",
                 MAC2STR(event->mac), event->aid);
    }
    else if (event_id == WIFI_EVENT_AP_STADISCONNECTED)
    {
        wifi_event_ap_stadisconnected_t *event = (wifi_event_ap_stadisconnected_t *)event_data;
        ESP_LOGI(TAG, "station " MACSTR " leave, AID=%d",
                 MAC2STR(event->mac), event->aid);

        // Inform that the controller is disconnected
        is_udp_console_connected = false;
        is_udp_controller_connected = false;
    }
}

/**
 * @brief Gets the data from the UDP server blocking until it receives data
 *
 * @param in Pointer to UDP packet to store the data
 * @return true
 * @return false
 */
bool wifiGetDataBlocking(UDPPacket *in)
{
    /* command step - receive  02  from udp rx queue */
    while (xQueueReceive(udp_data_rx, in, portMAX_DELAY) != pdTRUE)
    {
        vTaskDelay(1);
    }; // Don't return until we get some data on the UDP
    printf("Data obtained\n");

    return true;
};

/**
 * @brief Send data to the UDP server
 *
 * @param out UDP packet to send
 * @return true
 * @return false
 */
bool wifi_send_data(char *data)
{
    memcpy(out_packet.data, data, strlen(data));
    out_packet.size = strlen(data);
    if (xQueueSend(udp_data_tx, &out_packet, 2) != pdTRUE)
    {
        ESP_LOGE(TAG, "Error sending data to queue");
        return false;
    }
    return true;
}

/**
 * @brief Create the UDP server
 *
 * @param arg
 * @return esp_err_t
 */
esp_err_t udp_server_create(void *arg)
{
    if (is_udp_init)
    {
        return ESP_OK;
    }

    struct sockaddr_in *pdest_addr = &dest_addr;
    pdest_addr->sin_addr.s_addr = htonl(INADDR_ANY);
    pdest_addr->sin_family = AF_INET;
    pdest_addr->sin_port = htons(UDP_SERVER_PORT);

    sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
    if (sock < 0)
    {
        ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
        return ESP_FAIL;
    }
    ESP_LOGI(TAG, "Socket created");

    int err = bind(sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
    if (err < 0)
    {
        ESP_LOGE(TAG, "Socket unable to bind: errno %d", errno);
        return ESP_FAIL;
    }
    ESP_LOGI(TAG, "Socket bound, port %d", UDP_SERVER_PORT);

    is_udp_init = true;
    return ESP_OK;
}

/**
 * @brief Task to receive data from the UDP server
 *
 * @param pvParameters
 */
static void udp_server_rx_task(void *pvParameters)
{
    uint8_t cksum = 0;
    struct sockaddr_in source_addr;
    socklen_t socklen = sizeof(source_addr);

    while (1)
    {
        if (!is_udp_init)
        {
            vTaskDelay(20);
            continue;
        }
        vTaskDelay(10);

        int len = recvfrom(sock, rx_buffer, sizeof(rx_buffer) - 1, 0, (struct sockaddr *)&source_addr, &socklen);

        if (len < 0)
        {
            ESP_LOGE(TAG, "recvfrom failed: errno %d", errno);
            break;
        }
        else if (len > WIFI_RX_TX_PACKET_SIZE - 4)
        {
            ESP_LOGE(TAG, "Packet too large to process");
            continue;
        }
        else
        {
            rx_buffer[len] = 0; // Null-terminate whatever we received and treat like a string
#if DEBUG_UPD
            ESP_LOGI(TAG, "Received %d bytes:", len);
            for (size_t i = 0; i < len; i++)
            {
                printf(" data[%d]: %02x\n ", i, rx_buffer[i]);
            }
#endif
            memcpy(in_packet.data, rx_buffer, len);

            cksum = in_packet.data[len - 1];
            // remove cksum from packet
            in_packet.size = len - 1;

            // Check if is console device
            if (in_packet.data[0] == 0xff && in_packet.data[1] == 0x01)
            {
                if (in_packet.data[2] == 0x02 && in_packet.data[3] == 0x02)
                {
                    is_udp_console_connected = false;
                    ESP_LOGI(TAG, "Remote console closed");
                    continue;
                }
                ESP_LOGI(TAG, "Remote console detected");
                console_addr = source_addr;
                is_udp_console_connected = true;
                char *msg = "Connection accomplished";
                memcpy(out_packet.data, msg, strlen(msg));
                out_packet.size = strlen(msg);
                xQueueSend(udp_data_tx, &out_packet, 2);
                continue;
            }
            // Check if is controller device
            else if (in_packet.data[0] == 0x30)
            {
                is_udp_controller_connected = true;
                // check cksum
                if (cksum == calculate_cksum(in_packet.data, len - 1) && in_packet.size < 64)
                {
                    // ESP_LOGI(TAG, "Checksum OK");
                    if (xQueueSend(udp_data_rx, &in_packet, 2) != pdTRUE)
                    {
                        ESP_LOGE(TAG, "Error sending data to queue");
                    }
                }
                else
                {
                    ESP_LOGE(TAG, "Checksum error");
                    continue;
                }
            }
        }
    }
}

static void udp_server_tx_task(void *pvParameters)
{
    while (1)
    {
        if (!is_udp_init)
        {
            vTaskDelay(20);
            continue;
        }

        if ((xQueueReceive(udp_data_tx, &out_packet, 5) == pdTRUE) && is_udp_console_connected)
        {
            memcpy(tx_buffer, out_packet.data, out_packet.size);
            tx_buffer[out_packet.size] = calculate_cksum(tx_buffer, out_packet.size);
            tx_buffer[out_packet.size + 1] = 0;

            int err = sendto(sock, tx_buffer, out_packet.size + 1, 0, (struct sockaddr *)&console_addr, sizeof(console_addr));
            if (err < 0)
            {
                ESP_LOGE(TAG, "Error ocurred while sending.");
                continue;
            }
            else
            {
                // ESP_LOGI(TAG, "Package sent");
            }
        }
    }
}

/* PUBLIC FUNCTIONS */

/**
 * @brief Initialize the wifi module
 *
 */
void wifi_init()
{
    if (is_init)
    {
        return;
    }

    ESP_LOGI(TAG, "Initializing wifi");
    udp_data_rx = xQueueCreate(5, sizeof(UDPPacket));
    udp_data_tx = xQueueCreate(5, sizeof(UDPPacket));

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    ap_netif = esp_netif_create_default_wifi_ap();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        NULL));

    wifi_config_t wifi_config = {
        .ap = {
            .ssid = CONFIG_WIFI_BASE_SSID,
            .ssid_len = strlen(CONFIG_WIFI_BASE_SSID),
            .channel = CONFIG_WIFI_CHANNEL,
            .password = CONFIG_WIFI_PASSWORD,
            .max_connection = WIFI_MAX_STA_CONN,
            .authmode = WIFI_AUTH_WPA2_PSK,
        },
    };

    if (strlen(CONFIG_WIFI_PASSWORD) == 0)
    {
        wifi_config.ap.authmode = WIFI_AUTH_OPEN;
    }

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
    esp_wifi_set_channel(CONFIG_WIFI_CHANNEL, WIFI_SECOND_CHAN_NONE);

    esp_netif_ip_info_t ip_info = {
        .ip.addr = ipaddr_addr("192.168.43.42"),
        .netmask.addr = ipaddr_addr("255.255.255.0"),
        .gw.addr = ipaddr_addr("192.168.43.42"),
    };
    ESP_ERROR_CHECK(esp_netif_dhcps_stop(ap_netif));
    ESP_ERROR_CHECK(esp_netif_set_ip_info(ap_netif, &ip_info));
    ESP_ERROR_CHECK(esp_netif_dhcps_start(ap_netif));

    ESP_LOGI(TAG, "wifi_init_softap finished. SSID:%s password:%s channel:%d",
             CONFIG_WIFI_BASE_SSID, CONFIG_WIFI_PASSWORD, CONFIG_WIFI_CHANNEL);

    if (udp_server_create(NULL) == ESP_OK)
    {
        ESP_LOGI(TAG, "UDP server created");
    }
    else
    {
        ESP_LOGE(TAG, "Error creating UDP server");
    }

    xTaskCreate(udp_server_rx_task, "udp_rx_task", UDP_RX_TASK_STACKSIZE, NULL, UDP_RX_TASK_PRI, NULL);
    xTaskCreate(udp_server_tx_task, "udp_tx_task", UDP_TX_TASK_STACKSIZE, NULL, UDP_TX_TASK_PRI, NULL);

    is_init = true;
}
