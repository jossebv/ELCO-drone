/**
 * @file i2c_drv.c
 * @author Jose Manuel Bravo
 * @brief Functionality for the I2C driver.
 * @version 0.1
 * @date 2024-02-29
 *
 * @copyright Copyright (c) 2024
 *
 */

/* INCLUDES */
#include "i2c_drv.h"
#include "driver/i2c.h"

/* VARIABLES */
static bool is_init = false;

/* PUBLIC FUNCTIONS*/

/**
 * @brief Initializes the I2C driver
 *
 */
void i2c_drv_init()
{
    if (is_init)
    {
        return;
    }

    i2c_config_t i2c_conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_SDA_PIN,
        .scl_io_num = I2C_SCL_PIN,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ,
    };

    ESP_ERROR_CHECK(i2c_param_config(I2C_NUM_0, &i2c_conf));
    ESP_ERROR_CHECK(i2c_driver_install(I2C_NUM_0, I2C_MODE_MASTER, 0, 0, 0));

    is_init = true;
}