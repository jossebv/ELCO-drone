#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "motors.h"

void app_main(void)
{
    while (1)
    {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
