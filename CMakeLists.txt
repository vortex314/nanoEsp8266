# The following lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.5)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(nanoEsp8266 VERSION 0.1.0)

include(CTest)
enable_testing()

idf_component_register(SRCS "connect.c" "stdin_out.c"
                    INCLUDE_DIRS "include"
                    REQUIRES "tcpip_adapter")

add_executable(nanoEsp8266  main/main.cpp ) 

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
set(IDF_PATH /home/lieven/esp/ESP8266_RTOS_SDK )
set(GCC_FLAGS "-DANCHOR")
include(CPack)

add_definitions(${GCC_FLAGS})
include_directories(main ../Common ../ArduinoJson/src ../nanoAkka/main/ ../nanoAkka/components/wifi ${IDF_PATH}/components/freertos/include )
target_link_libraries(nanoEsp8266 -lCommon  )
