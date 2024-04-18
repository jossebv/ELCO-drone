/**
 * @file adc.c
 * @author Jose Manuel Bravo
 * @brief
 * @version 0.1
 * @date 2024-04-18
 *
 * @copyright Copyright (c) 2024
 *
 */

#include "adc.h"

#include "driver/adc.h"

#define MAX_V 3300
#define NO_OF_SAMPLES 64

static const adc_atten_t atten = ADC_ATTEN_DB_11;
static const adc_bits_width_t width = ADC_WIDTH_BIT_12;

uint32_t adc_read_voltage()
{
    // Read ADC and obtain voltage
    uint32_t adc_reading = 0;
    for (int i = 0; i < NO_OF_SAMPLES; i++)
    {
        adc_reading += adc1_get_raw(ADC1_CHANNEL_5);
    }
    adc_reading /= NO_OF_SAMPLES;
    uint32_t voltage = adc_reading * MAX_V / 4095;
    return voltage;
}

void adc_init(void)
{
    // Configure ADC
    adc1_config_width(width);
    adc1_config_channel_atten(ADC1_CHANNEL_5, atten);
}