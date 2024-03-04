idf_component_register(SRCS "system.c" "system_fsm.c"
                       INCLUDE_DIRS "." 
                       REQUIRES i2c_drv sensors fsm)