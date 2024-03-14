idf_component_register(SRCS "motors.c"
                       INCLUDE_DIRS "."
                       REQUIRES driver esp_timer pid_control controller sensors)