idf_component_register(SRCS "system.c"
                       INCLUDE_DIRS "." 
                       REQUIRES i2c_drv sensors)