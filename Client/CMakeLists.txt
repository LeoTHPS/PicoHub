cmake_minimum_required(VERSION 3.24)

include(../PicoHub.cmake)

add_subdirectory(../API PicoHubAPI)

project(PicoHubClient)

add_executable(PicoHubClient main.cpp)
target_link_libraries(PicoHubClient PRIVATE PicoHubAPI)
set_target_properties(PicoHubClient PROPERTIES OUTPUT_NAME "PicoHub")
