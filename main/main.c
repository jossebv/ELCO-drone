#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "system.h"
#include "motors.h"

void app_main(void)
{
    system_init();

    while (1)
    {
        print_hola();
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
