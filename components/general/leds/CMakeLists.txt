idf_component_register(SRCS "led.c" "led_fsm.c"
                       INCLUDE_DIRS "."
                       REQUIRES driver esp_timer fsm)