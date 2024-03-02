idf_component_register(SRCS "sensors.c"
                       INCLUDE_DIRS "." "../../../main"
                       REQUIRES mpu6050 comb_filter)