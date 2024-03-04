# The following five lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.16)

set(EXTRA_COMPONENT_DIRS "${EXTRA_COMPONENT_DIRS}" 
        "./components/general/comb_filter"
        "./components/general/motors"
        "./components/general/sensors"
        "./components/drivers/i2c_drv" 
        "./components/drivers/fsm" 
        "./components/drivers/mpu6050" 
        "./components/system")

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(ELCO-drone)
