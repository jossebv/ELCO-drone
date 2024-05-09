/**
 * @file i2c_drv.h
 * @author Jose Manuel Bravo
 * @brief Header file for I2C driver
 * @version 0.1
 * @date 2024-02-29
 *
 * @copyright Copyright (c) 2024
 *
 */

#ifndef I2C_DRV_H
#define I2C_DRV_H

/* DEFINES */
#define I2C_MASTER_FREQ_HZ 400000 /**< I2C master clock frequency */
#define I2C_SDA_PIN 21            /**< Pin for SDA communication */
#define I2C_SCL_PIN 22            /**< Pin for SCL communication */

/* PUBLIC FUNCTIONS */
void i2c_drv_init();

#endif // I2C_DRV_H