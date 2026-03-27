cmake_minimum_required(VERSION 3.22)

project(robot_platform C ASM)

option(PLATFORM_TARGET_HW "Build hardware target" ON)
option(PLATFORM_TARGET_REPLAY "Build replay target" OFF)
option(PLATFORM_TARGET_SIM "Build simulation target" OFF)

set(PLATFORM_ROOT ${CMAKE_CURRENT_LIST_DIR})
set(PLATFORM_PROJECTS_DIR ${PLATFORM_ROOT}/projects)
set(PLATFORM_RUNTIME_DIR ${PLATFORM_ROOT}/runtime)
set(PLATFORM_TOOLS_DIR ${PLATFORM_ROOT}/tools)
set(PLATFORM_REPO_ROOT ${PLATFORM_ROOT}/..)

message(STATUS "Configuring robot_platform")
message(STATUS "PLATFORM_ROOT=${PLATFORM_ROOT}")
message(STATUS "PLATFORM_TARGET_HW=${PLATFORM_TARGET_HW}")
message(STATUS "PLATFORM_TARGET_REPLAY=${PLATFORM_TARGET_REPLAY}")
message(STATUS "PLATFORM_TARGET_SIM=${PLATFORM_TARGET_SIM}")

add_custom_target(platform_info
  COMMAND ${CMAKE_COMMAND} -E echo "robot_platform bootstrap ready"
  COMMAND ${CMAKE_COMMAND} -E echo "Phase 2 seeds a GCC-compilable hardware target from generated code"
)

if(NOT PLATFORM_TARGET_HW)
  return()
endif()

set(CHASSIS_ROOT ${PLATFORM_REPO_ROOT}/Chassis)
set(CHASSIS_DRIVERS_DIR ${CHASSIS_ROOT}/Drivers)
set(CHASSIS_HAL_DIR ${CHASSIS_DRIVERS_DIR}/STM32H7xx_HAL_Driver)
set(CHASSIS_CMSIS_DIR ${CHASSIS_DRIVERS_DIR}/CMSIS)
set(CHASSIS_FREERTOS_DIR ${CHASSIS_ROOT}/Middlewares/Third_Party/FreeRTOS/Source)
set(GENERATED_RAW_DIR ${PLATFORM_RUNTIME_DIR}/generated/stm32h7_ctrl_board_raw)
set(PLATFORM_THIRD_PARTY_DIR ${PLATFORM_ROOT}/third_party)
set(PLATFORM_FREERTOS_PORT_DIR ${PLATFORM_THIRD_PARTY_DIR}/freertos_port_gcc_cm7_r0p1)
set(PLATFORM_LINKER_SCRIPT ${PLATFORM_ROOT}/cmake/linker/stm32h723_flash.ld)
set(PLATFORM_BOARD_BSP_DIR ${PLATFORM_RUNTIME_DIR}/bsp/boards/stm32h7_ctrl_board)
set(PLATFORM_REMOTE_CONTROL_DIR ${PLATFORM_RUNTIME_DIR}/bsp/devices/remote_control)
set(PLATFORM_DM_MOTOR_DIR ${PLATFORM_RUNTIME_DIR}/bsp/devices/dm_motor)
set(PLATFORM_SEED_APP_DIR ${PLATFORM_RUNTIME_DIR}/app/balance_chassis_seed)
set(PLATFORM_LEGACY_APP_DIR ${PLATFORM_RUNTIME_DIR}/app/balance_chassis)
set(PLATFORM_BMI088_DIR ${PLATFORM_RUNTIME_DIR}/bsp/devices/bmi088)
set(PLATFORM_DSP_LIB ${CHASSIS_CMSIS_DIR}/DSP/Lib/GCC/libarm_cortexM7lfdp_math.a)

set(BALANCE_CHASSIS_CPU_FLAGS
  -mcpu=cortex-m7
  -mthumb
  -mfpu=fpv5-d16
  -mfloat-abi=hard
)

set(BALANCE_CHASSIS_COMMON_FLAGS
  ${BALANCE_CHASSIS_CPU_FLAGS}
  -ffunction-sections
  -fdata-sections
  -fno-common
)

set(BALANCE_CHASSIS_GENERATED_SOURCES
  ${GENERATED_RAW_DIR}/Src/dma.c
  ${GENERATED_RAW_DIR}/Src/fdcan.c
  ${GENERATED_RAW_DIR}/Src/gpio.c
  ${GENERATED_RAW_DIR}/Src/spi.c
  ${GENERATED_RAW_DIR}/Src/stm32h7xx_hal_msp.c
  ${GENERATED_RAW_DIR}/Src/stm32h7xx_hal_timebase_tim.c
  ${GENERATED_RAW_DIR}/Src/stm32h7xx_it.c
  ${GENERATED_RAW_DIR}/Src/tim.c
  ${GENERATED_RAW_DIR}/Src/usart.c
  ${CHASSIS_ROOT}/Core/Src/system_stm32h7xx.c
)

set(BALANCE_CHASSIS_HAL_SOURCES
  ${CHASSIS_HAL_DIR}/Src/stm32h7xx_hal.c
  ${CHASSIS_HAL_DIR}/Src/stm32h7xx_hal_cortex.c
  ${CHASSIS_HAL_DIR}/Src/stm32h7xx_hal_dma.c
  ${CHASSIS_HAL_DIR}/Src/stm32h7xx_hal_dma_ex.c
  ${CHASSIS_HAL_DIR}/Src/stm32h7xx_hal_exti.c
  ${CHASSIS_HAL_DIR}/Src/stm32h7xx_hal_fdcan.c
  ${CHASSIS_HAL_DIR}/Src/stm32h7xx_hal_flash.c
  ${CHASSIS_HAL_DIR}/Src/stm32h7xx_hal_flash_ex.c
  ${CHASSIS_HAL_DIR}/Src/stm32h7xx_hal_gpio.c
  ${CHASSIS_HAL_DIR}/Src/stm32h7xx_hal_hsem.c
  ${CHASSIS_HAL_DIR}/Src/stm32h7xx_hal_i2c.c
  ${CHASSIS_HAL_DIR}/Src/stm32h7xx_hal_i2c_ex.c
  ${CHASSIS_HAL_DIR}/Src/stm32h7xx_hal_mdma.c
  ${CHASSIS_HAL_DIR}/Src/stm32h7xx_hal_pwr.c
  ${CHASSIS_HAL_DIR}/Src/stm32h7xx_hal_pwr_ex.c
  ${CHASSIS_HAL_DIR}/Src/stm32h7xx_hal_rcc.c
  ${CHASSIS_HAL_DIR}/Src/stm32h7xx_hal_rcc_ex.c
  ${CHASSIS_HAL_DIR}/Src/stm32h7xx_hal_spi.c
  ${CHASSIS_HAL_DIR}/Src/stm32h7xx_hal_spi_ex.c
  ${CHASSIS_HAL_DIR}/Src/stm32h7xx_hal_tim.c
  ${CHASSIS_HAL_DIR}/Src/stm32h7xx_hal_tim_ex.c
  ${CHASSIS_HAL_DIR}/Src/stm32h7xx_hal_uart.c
  ${CHASSIS_HAL_DIR}/Src/stm32h7xx_hal_uart_ex.c
)

set(BALANCE_CHASSIS_BSP_SEED_SOURCES
  ${PLATFORM_BOARD_BSP_DIR}/bsp_dwt.c
  ${PLATFORM_BOARD_BSP_DIR}/bsp_PWM.c
  ${PLATFORM_BOARD_BSP_DIR}/bsp_uart.c
  ${PLATFORM_BOARD_BSP_DIR}/can_bsp.c
  ${PLATFORM_REMOTE_CONTROL_DIR}/remote_control.c
  ${PLATFORM_DM_MOTOR_DIR}/dm4310_drv.c
  ${PLATFORM_SEED_APP_DIR}/legacy_seed_state.c
)

set(BALANCE_CHASSIS_BSP_FULL_SOURCES
  ${PLATFORM_BOARD_BSP_DIR}/bsp_dwt.c
  ${PLATFORM_BOARD_BSP_DIR}/bsp_PWM.c
  ${PLATFORM_BOARD_BSP_DIR}/bsp_uart.c
  ${PLATFORM_BOARD_BSP_DIR}/can_bsp.c
  ${PLATFORM_DM_MOTOR_DIR}/dm4310_drv.c
)

add_library(balance_chassis_generated_raw STATIC
  ${BALANCE_CHASSIS_GENERATED_SOURCES}
  ${BALANCE_CHASSIS_HAL_SOURCES}
)

target_include_directories(balance_chassis_generated_raw
  PUBLIC
    ${GENERATED_RAW_DIR}/Inc
    ${CHASSIS_HAL_DIR}/Inc
    ${CHASSIS_CMSIS_DIR}/Device/ST/STM32H7xx/Include
    ${CHASSIS_CMSIS_DIR}/Include
    ${CHASSIS_CMSIS_DIR}/DSP/Include
    ${PLATFORM_BOARD_BSP_DIR}
    ${PLATFORM_REMOTE_CONTROL_DIR}
    ${PLATFORM_DM_MOTOR_DIR}
    ${PLATFORM_SEED_APP_DIR}
)

target_compile_definitions(balance_chassis_generated_raw
  PUBLIC
    USE_HAL_DRIVER
    STM32H723xx
)

target_compile_options(balance_chassis_generated_raw
  PRIVATE
    ${BALANCE_CHASSIS_COMMON_FLAGS}
    -Wall
    -Wextra
    -Wno-unused-parameter
)

add_library(balance_chassis_bsp_seed STATIC
  ${BALANCE_CHASSIS_BSP_SEED_SOURCES}
)

target_link_libraries(balance_chassis_bsp_seed
  PUBLIC
    balance_chassis_generated_raw
)

target_include_directories(balance_chassis_bsp_seed
  PUBLIC
    ${PLATFORM_BOARD_BSP_DIR}
    ${PLATFORM_REMOTE_CONTROL_DIR}
    ${PLATFORM_DM_MOTOR_DIR}
    ${PLATFORM_SEED_APP_DIR}
)

target_compile_options(balance_chassis_bsp_seed
  PRIVATE
    ${BALANCE_CHASSIS_COMMON_FLAGS}
    -Wall
    -Wextra
    -Wno-unused-parameter
)

set(BALANCE_CHASSIS_FREERTOS_SOURCES
  ${CHASSIS_FREERTOS_DIR}/CMSIS_RTOS/cmsis_os.c
  ${CHASSIS_FREERTOS_DIR}/croutine.c
  ${CHASSIS_FREERTOS_DIR}/event_groups.c
  ${CHASSIS_FREERTOS_DIR}/list.c
  ${CHASSIS_FREERTOS_DIR}/portable/MemMang/heap_4.c
  ${PLATFORM_FREERTOS_PORT_DIR}/port.c
  ${CHASSIS_FREERTOS_DIR}/queue.c
  ${CHASSIS_FREERTOS_DIR}/stream_buffer.c
  ${CHASSIS_FREERTOS_DIR}/tasks.c
  ${CHASSIS_FREERTOS_DIR}/timers.c
)

set(BALANCE_CHASSIS_LEGACY_APP_SOURCES
  ${PLATFORM_LEGACY_APP_DIR}/INS_task.c
  ${PLATFORM_LEGACY_APP_DIR}/chassis_task.c
  ${PLATFORM_LEGACY_APP_DIR}/motor_control_task.c
  ${PLATFORM_LEGACY_APP_DIR}/observe_task.c
  ${PLATFORM_LEGACY_APP_DIR}/remote_control.c
  ${PLATFORM_LEGACY_APP_DIR}/remote_task.c
  ${PLATFORM_RUNTIME_DIR}/module/algorithm/EKF/QuaternionEKF.c
  ${PLATFORM_RUNTIME_DIR}/module/algorithm/PID/pid.c
  ${PLATFORM_RUNTIME_DIR}/module/algorithm/VMC/VMC_calc.c
  ${PLATFORM_RUNTIME_DIR}/module/algorithm/kalman/kalman_filter.c
  ${PLATFORM_RUNTIME_DIR}/module/algorithm/mahony/mahony_filter.c
  ${PLATFORM_RUNTIME_DIR}/module/controller/controller.c
  ${PLATFORM_RUNTIME_DIR}/module/lib/user_lib.c
  ${PLATFORM_RUNTIME_DIR}/module/message_center/message_center.c
  ${PLATFORM_BMI088_DIR}/BMI088Middleware.c
  ${PLATFORM_BMI088_DIR}/BMI088driver.c
)

add_executable(balance_chassis_hw_seed.elf
  ${GENERATED_RAW_DIR}/Src/main.c
  ${GENERATED_RAW_DIR}/Src/freertos.c
  ${CHASSIS_ROOT}/Drivers/CMSIS/Device/ST/STM32H7xx/Source/Templates/gcc/startup_stm32h723xx.s
  ${BALANCE_CHASSIS_FREERTOS_SOURCES}
)

add_library(balance_chassis_legacy_full_obj OBJECT
  ${BALANCE_CHASSIS_LEGACY_APP_SOURCES}
)

target_include_directories(balance_chassis_legacy_full_obj
  PRIVATE
    ${GENERATED_RAW_DIR}/Inc
    ${CHASSIS_HAL_DIR}/Inc
    ${CHASSIS_CMSIS_DIR}/Device/ST/STM32H7xx/Include
    ${CHASSIS_CMSIS_DIR}/Include
    ${CHASSIS_CMSIS_DIR}/DSP/Include
    ${CHASSIS_FREERTOS_DIR}/include
    ${CHASSIS_FREERTOS_DIR}/CMSIS_RTOS
    ${PLATFORM_FREERTOS_PORT_DIR}
    ${PLATFORM_BOARD_BSP_DIR}
    ${PLATFORM_DM_MOTOR_DIR}
    ${PLATFORM_SEED_APP_DIR}
    ${PLATFORM_LEGACY_APP_DIR}
    ${PLATFORM_RUNTIME_DIR}/module/algorithm/EKF
    ${PLATFORM_RUNTIME_DIR}/module/algorithm/PID
    ${PLATFORM_RUNTIME_DIR}/module/algorithm/VMC
    ${PLATFORM_RUNTIME_DIR}/module/algorithm/kalman
    ${PLATFORM_RUNTIME_DIR}/module/algorithm/mahony
    ${PLATFORM_RUNTIME_DIR}/module/controller
    ${PLATFORM_RUNTIME_DIR}/module/lib
    ${PLATFORM_RUNTIME_DIR}/module/message_center
    ${PLATFORM_BMI088_DIR}
    ${PLATFORM_REMOTE_CONTROL_DIR}
)

target_compile_definitions(balance_chassis_legacy_full_obj
  PRIVATE
    USE_HAL_DRIVER
    STM32H723xx
)

target_compile_options(balance_chassis_legacy_full_obj
  PRIVATE
    ${BALANCE_CHASSIS_COMMON_FLAGS}
    -Wall
    -Wextra
    -Wno-unused-parameter
)

target_link_libraries(balance_chassis_hw_seed.elf
  PRIVATE
    balance_chassis_generated_raw
    balance_chassis_bsp_seed
)

target_include_directories(balance_chassis_hw_seed.elf
  PRIVATE
    ${CHASSIS_FREERTOS_DIR}/include
    ${CHASSIS_FREERTOS_DIR}/CMSIS_RTOS
    ${PLATFORM_FREERTOS_PORT_DIR}
)

target_compile_definitions(balance_chassis_hw_seed.elf
  PRIVATE
    USE_HAL_DRIVER
    STM32H723xx
)

target_compile_options(balance_chassis_hw_seed.elf
  PRIVATE
    ${BALANCE_CHASSIS_COMMON_FLAGS}
    -Wall
    -Wextra
    -Wno-unused-parameter
)

target_link_options(balance_chassis_hw_seed.elf
  PRIVATE
    ${BALANCE_CHASSIS_CPU_FLAGS}
    -T${PLATFORM_LINKER_SCRIPT}
    -Wl,--gc-sections
    -Wl,-Map=${CMAKE_CURRENT_BINARY_DIR}/balance_chassis_hw_seed.map
    --specs=nano.specs
    --specs=nosys.specs
)

add_custom_command(TARGET balance_chassis_hw_seed.elf POST_BUILD
  COMMAND ${CMAKE_SIZE} $<TARGET_FILE:balance_chassis_hw_seed.elf>
  COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:balance_chassis_hw_seed.elf> ${CMAKE_CURRENT_BINARY_DIR}/balance_chassis_hw_seed.bin
)

add_library(balance_chassis_bsp_full STATIC
  ${BALANCE_CHASSIS_BSP_FULL_SOURCES}
)

target_link_libraries(balance_chassis_bsp_full
  PUBLIC
    balance_chassis_generated_raw
)

target_include_directories(balance_chassis_bsp_full
  PUBLIC
    ${PLATFORM_BOARD_BSP_DIR}
    ${PLATFORM_DM_MOTOR_DIR}
    ${PLATFORM_LEGACY_APP_DIR}
    ${PLATFORM_REMOTE_CONTROL_DIR}
)

target_compile_options(balance_chassis_bsp_full
  PRIVATE
    ${BALANCE_CHASSIS_COMMON_FLAGS}
    -Wall
    -Wextra
    -Wno-unused-parameter
)

add_executable(balance_chassis_legacy_full.elf
  ${GENERATED_RAW_DIR}/Src/main.c
  ${PLATFORM_LEGACY_APP_DIR}/freertos_legacy.c
  ${CHASSIS_ROOT}/Drivers/CMSIS/Device/ST/STM32H7xx/Source/Templates/gcc/startup_stm32h723xx.s
  ${BALANCE_CHASSIS_FREERTOS_SOURCES}
  $<TARGET_OBJECTS:balance_chassis_legacy_full_obj>
)

target_link_libraries(balance_chassis_legacy_full.elf
  PRIVATE
    balance_chassis_generated_raw
    balance_chassis_bsp_full
    ${PLATFORM_DSP_LIB}
    m
)

target_include_directories(balance_chassis_legacy_full.elf
  PRIVATE
    ${CHASSIS_FREERTOS_DIR}/include
    ${CHASSIS_FREERTOS_DIR}/CMSIS_RTOS
    ${PLATFORM_FREERTOS_PORT_DIR}
    ${PLATFORM_BOARD_BSP_DIR}
    ${PLATFORM_LEGACY_APP_DIR}
    ${PLATFORM_REMOTE_CONTROL_DIR}
)

target_compile_definitions(balance_chassis_legacy_full.elf
  PRIVATE
    USE_HAL_DRIVER
    STM32H723xx
)

target_compile_options(balance_chassis_legacy_full.elf
  PRIVATE
    ${BALANCE_CHASSIS_COMMON_FLAGS}
    -Wall
    -Wextra
    -Wno-unused-parameter
)

target_link_options(balance_chassis_legacy_full.elf
  PRIVATE
    ${BALANCE_CHASSIS_CPU_FLAGS}
    -T${PLATFORM_LINKER_SCRIPT}
    -Wl,--gc-sections
    -Wl,-Map=${CMAKE_CURRENT_BINARY_DIR}/balance_chassis_legacy_full.map
    --specs=nano.specs
    --specs=nosys.specs
)

add_custom_command(TARGET balance_chassis_legacy_full.elf POST_BUILD
  COMMAND ${CMAKE_SIZE} $<TARGET_FILE:balance_chassis_legacy_full.elf>
  COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:balance_chassis_legacy_full.elf> ${CMAKE_CURRENT_BINARY_DIR}/balance_chassis_legacy_full.bin
)
