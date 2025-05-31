cmake_minimum_required(VERSION 3.24)

include(../PicoHub.cmake)

project(PicoHubAPI)

add_library(PicoHubAPI SHARED PicoHub.cpp)
target_compile_definitions(PicoHubAPI PRIVATE -D_PICO_HUB_API=1)
target_include_directories(PicoHubAPI PUBLIC ${CMAKE_CURRENT_LIST_DIR} ${CMAKE_CURRENT_LIST_DIR}/../)
set_target_properties(PicoHubAPI PROPERTIES PREFIX "" OUTPUT_NAME "PicoHub.API")

if(UNIX)
	target_link_libraries(PicoHubAPI PUBLIC )
	target_compile_definitions(PicoHubAPI PUBLIC -DPICO_HUB_API_UNIX=1)
elseif(WIN32)
	target_link_libraries(PicoHubAPI PUBLIC )
	target_compile_definitions(PicoHubAPI PUBLIC -DPICO_HUB_API_WIN32=1)
endif()
