cmake_minimum_required(VERSION 3.24)

if(NOT DEFINED PICO_BOARD)
	set(PICO_BOARD pico_w)
endif()

include(../PicoHub.cmake)
include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

project(PicoHub)

pico_sdk_init()

add_executable(PicoHub
	main.cpp

	PicoHub.cpp
)

target_link_libraries(PicoHub
	pico_stdlib
	pico_platform
	pico_unique_id

	hardware_adc
	hardware_i2c
	hardware_pwm
	hardware_spi
	hardware_gpio
	hardware_uart
	hardware_vreg
	hardware_timer
	hardware_clocks
	hardware_resets
)

target_compile_definitions(PicoHub PRIVATE
	-DPICO_CYW43_ARCH_DEFAULT_COUNTRY_CODE=${PICO_CYW43_ARCH_DEFAULT_COUNTRY_CODE}
)

if(PICO_BOARD STREQUAL pico_w)
	target_link_libraries(PicoHub pico_lwip pico_cyw43_arch_lwip_threadsafe_background)
	target_include_directories(PicoHub PRIVATE ${CMAKE_CURRENT_LIST_DIR})
endif()

pico_add_extra_outputs(PicoHub)
pico_enable_stdio_usb(PicoHub 1)
pico_enable_stdio_uart(PicoHub 0)
