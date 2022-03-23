cmake_minimum_required(VERSION 3.13)

# Pull in SDK (must be before project)
include(pico_sdk_import.cmake)

# Pull in FreeRTOS
include(FreeRTOS_Kernel_import.cmake)

project(app_console C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)



pico_sdk_init()

file(GLOB_RECURSE SOURCES   "console/*.c" "log/*.c" )
add_executable(app_console
        main.c
        appConsole.c
        ${SOURCES}
        )


        target_compile_definitions(app_console PRIVATE
        CONFIG_LOG_DEFAULT_LEVEL=LOG_DEBUG
                )
target_include_directories(app_console PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/../../Common/include)

include_directories(console log)

target_link_libraries(app_console pico_stdlib FreeRTOS-Kernel FreeRTOS-Kernel-Heap1 )
pico_add_extra_outputs(app_console)
# Enable usb output, disable uart output
pico_enable_stdio_usb(app_console 1)
pico_enable_stdio_uart(app_console 0)