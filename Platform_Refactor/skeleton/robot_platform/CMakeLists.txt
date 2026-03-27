cmake_minimum_required(VERSION 3.22)

project(robot_platform C CXX ASM)

option(PLATFORM_TARGET_HW "Build hardware firmware target" ON)
option(PLATFORM_TARGET_REPLAY "Build replay target" OFF)
option(PLATFORM_TARGET_SIM "Build physics sim target" OFF)

message(STATUS "robot_platform skeleton loaded")
message(STATUS "PLATFORM_TARGET_HW=${PLATFORM_TARGET_HW}")
message(STATUS "PLATFORM_TARGET_REPLAY=${PLATFORM_TARGET_REPLAY}")
message(STATUS "PLATFORM_TARGET_SIM=${PLATFORM_TARGET_SIM}")

add_custom_target(platform_info
  COMMAND ${CMAKE_COMMAND} -E echo "Platform skeleton only. No firmware target wired yet."
)

