/**
 * @file adc.h
 * @author Jose Manuel Bravo
 * @brief
 * @version 0.1
 * @date 2024-04-18
 *
 * @copyright Copyright (c) 2024
 *
 */

#ifndef __ADC_H__
#define __ADC_H__

#include <stdint.h>

void adc_init(void);
uint32_t adc_read_voltage(void);

#endif // __ADC_H__