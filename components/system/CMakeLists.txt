idf_component_register(SRCS "system.c" "system_fsm.c"
                       INCLUDE_DIRS "." "../../main"
                       REQUIRES i2c_drv sensors fsm esp_timer wifi nvs_flash controller motors leds adc comms)