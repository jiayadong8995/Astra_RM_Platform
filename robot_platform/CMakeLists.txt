cmake_minimum_required(VERSION 3.22)

project(robot_platform C ASM)

option(PLATFORM_TARGET_HW "Build hardware target" ON)
option(PLATFORM_TARGET_REPLAY "Build replay target" OFF)
option(PLATFORM_TARGET_SIM "Build simulation target" OFF)
option(PLATFORM_SIM_ASAN "Enable AddressSanitizer for SITL target" OFF)

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
message(STATUS "PLATFORM_SIM_ASAN=${PLATFORM_SIM_ASAN}")

add_custom_target(platform_info
  COMMAND ${CMAKE_COMMAND} -E echo "robot_platform bootstrap ready"
  COMMAND ${CMAKE_COMMAND} -E echo "Phase 2 seeds a GCC-compilable hardware target from generated code"
)

set(PLATFORM_THIRD_PARTY_DIR ${PLATFORM_ROOT}/third_party)
set(CHASSIS_ROOT ${PLATFORM_THIRD_PARTY_DIR}/stm32_cube)
set(CHASSIS_DRIVERS_DIR ${CHASSIS_ROOT}/Drivers)
set(CHASSIS_HAL_DIR ${CHASSIS_DRIVERS_DIR}/STM32H7xx_HAL_Driver)
set(CHASSIS_CMSIS_DIR ${CHASSIS_DRIVERS_DIR}/CMSIS)
set(CHASSIS_FREERTOS_DIR ${CHASSIS_ROOT}/Middlewares/Third_Party/FreeRTOS/Source)
set(GENERATED_RAW_DIR ${PLATFORM_RUNTIME_DIR}/generated/stm32h7_ctrl_board_raw)
set(PLATFORM_FREERTOS_PORT_DIR ${PLATFORM_THIRD_PARTY_DIR}/freertos_port_gcc_cm7_r0p1)
set(PLATFORM_LINKER_SCRIPT ${PLATFORM_ROOT}/cmake/linker/stm32h723_flash.ld)
set(PLATFORM_BOARD_BSP_DIR ${PLATFORM_RUNTIME_DIR}/bsp/boards/stm32h7_ctrl_board)
set(PLATFORM_REMOTE_CONTROL_DIR ${PLATFORM_RUNTIME_DIR}/bsp/devices/remote_control)
set(PLATFORM_DM_MOTOR_DIR ${PLATFORM_RUNTIME_DIR}/bsp/devices/dm_motor)
set(PLATFORM_DEVICE_DIR ${PLATFORM_RUNTIME_DIR}/device)
set(PLATFORM_LEGACY_APP_DIR ${PLATFORM_RUNTIME_DIR}/app/balance_chassis)
set(PLATFORM_RUNTIME_SERVICE_DIR ${PLATFORM_RUNTIME_DIR}/runtime_service)
set(PLATFORM_CONTROL_CONTRACT_DIR ${PLATFORM_RUNTIME_DIR}/control/contracts)
set(PLATFORM_BMI088_DIR ${PLATFORM_RUNTIME_DIR}/bsp/devices/bmi088)

set(BALANCE_CHASSIS_DEVICE_SOURCES)

if(PLATFORM_TARGET_HW)
  list(APPEND BALANCE_CHASSIS_DEVICE_SOURCES
    ${PLATFORM_DEVICE_DIR}/imu/bmi088_device_hw.c
    ${PLATFORM_DEVICE_DIR}/remote/dbus_remote_device_hw.c
    ${PLATFORM_DEVICE_DIR}/actuator/motor/motor_actuator_device_hw.c
  )
endif()

if(PLATFORM_TARGET_SIM)
  list(APPEND BALANCE_CHASSIS_DEVICE_SOURCES
    ${PLATFORM_DEVICE_DIR}/imu/bmi088_device_sitl.c
    ${PLATFORM_DEVICE_DIR}/remote/dbus_remote_device_sitl.c
    ${PLATFORM_DEVICE_DIR}/actuator/motor/motor_actuator_device_sitl.c
  )
endif()
if(PLATFORM_TARGET_HW)


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
  ${CHASSIS_CMSIS_DIR}/Device/ST/STM32H7xx/Source/Templates/system_stm32h7xx.c
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
  ${PLATFORM_REMOTE_CONTROL_DIR}/rc_input_bridge.c
  ${PLATFORM_REMOTE_CONTROL_DIR}/remote_control.c
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
    ${PLATFORM_CONTROL_CONTRACT_DIR}
    ${PLATFORM_LEGACY_APP_DIR}
    ${CHASSIS_FREERTOS_DIR}/include
    ${CHASSIS_FREERTOS_DIR}/CMSIS_RTOS
    ${PLATFORM_FREERTOS_PORT_DIR}
    ${PLATFORM_RUNTIME_DIR}/module/message_center
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

add_executable(balance_chassis_hw_seed.elf
  ${GENERATED_RAW_DIR}/Src/main.c
  ${GENERATED_RAW_DIR}/Src/freertos.c
  ${CHASSIS_ROOT}/Drivers/CMSIS/Device/ST/STM32H7xx/Source/Templates/gcc/startup_stm32h723xx.s
  ${BALANCE_CHASSIS_FREERTOS_SOURCES}
)

target_link_libraries(balance_chassis_hw_seed.elf
  PRIVATE
    balance_chassis_generated_raw
    balance_chassis_bsp_seed
    balance_chassis_device
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

endif() # end of PLATFORM_TARGET_HW

if(PLATFORM_TARGET_SIM)
  message(STATUS "Configuring SITL target balance_chassis_sitl")
  
  set(PLATFORM_SITL_BSP_DIR ${PLATFORM_RUNTIME_DIR}/bsp/sitl)
  set(PLATFORM_FREERTOS_PORT_DIR ${PLATFORM_THIRD_PARTY_DIR}/freertos_port_linux)

  set(BALANCE_CHASSIS_FREERTOS_SOURCES
    ${CHASSIS_FREERTOS_DIR}/CMSIS_RTOS/cmsis_os.c
    ${CHASSIS_FREERTOS_DIR}/croutine.c
    ${CHASSIS_FREERTOS_DIR}/event_groups.c
    ${CHASSIS_FREERTOS_DIR}/list.c
    ${CHASSIS_FREERTOS_DIR}/portable/MemMang/heap_3.c
    ${PLATFORM_FREERTOS_PORT_DIR}/port.c
    ${PLATFORM_FREERTOS_PORT_DIR}/utils/wait_for_event.c
    ${CHASSIS_FREERTOS_DIR}/queue.c
    ${CHASSIS_FREERTOS_DIR}/stream_buffer.c
    ${CHASSIS_FREERTOS_DIR}/tasks.c
    ${CHASSIS_FREERTOS_DIR}/timers.c
  )

  set(BALANCE_CHASSIS_SITL_BSP_SOURCES
    ${PLATFORM_SITL_BSP_DIR}/hal_stub.c
    ${PLATFORM_SITL_BSP_DIR}/bsp_dwt_sitl.c
    ${PLATFORM_SITL_BSP_DIR}/bsp_PWM_sitl.c
    ${PLATFORM_SITL_BSP_DIR}/can_bsp_sitl.c
    ${PLATFORM_SITL_BSP_DIR}/BMI088driver_sitl.c
    ${PLATFORM_SITL_BSP_DIR}/dm4310_drv_sitl.c
    ${PLATFORM_REMOTE_CONTROL_DIR}/remote_control.c
    ${PLATFORM_REMOTE_CONTROL_DIR}/rc_input_bridge.c
  )

  set(BALANCE_CHASSIS_LEGACY_APP_SOURCES
    ${PLATFORM_LEGACY_APP_DIR}/legacy/chassis_task.c
    ${PLATFORM_LEGACY_APP_DIR}/app_bringup/task_registry.c
    ${PLATFORM_LEGACY_APP_DIR}/app_flow/chassis_orchestration.c
    ${PLATFORM_LEGACY_APP_DIR}/app_flow/remote_orchestration.c
    ${PLATFORM_LEGACY_APP_DIR}/app_io/chassis_topics.c
    ${PLATFORM_LEGACY_APP_DIR}/app_io/remote_topics.c
    ${PLATFORM_RUNTIME_SERVICE_DIR}/control_runtime/chassis_control_support.c
    ${PLATFORM_RUNTIME_SERVICE_DIR}/sensing/INS_task.c
    ${PLATFORM_RUNTIME_SERVICE_DIR}/sensing/ins_runtime.c
    ${PLATFORM_RUNTIME_SERVICE_DIR}/sensing/ins_topics.c
    ${PLATFORM_RUNTIME_SERVICE_DIR}/observe/observe_orchestration.c
    ${PLATFORM_RUNTIME_SERVICE_DIR}/observe/observe_topics.c
    ${PLATFORM_RUNTIME_SERVICE_DIR}/observe/observe_task.c
    ${PLATFORM_RUNTIME_SERVICE_DIR}/actuator/actuator_runtime.c
    ${PLATFORM_RUNTIME_SERVICE_DIR}/actuator/actuator_topics.c
    ${PLATFORM_RUNTIME_SERVICE_DIR}/actuator/motor_control_task.c
    ${PLATFORM_LEGACY_APP_DIR}/legacy/remote_task.c
    ${PLATFORM_RUNTIME_DIR}/module/algorithm/EKF/QuaternionEKF.c
    ${PLATFORM_RUNTIME_DIR}/module/algorithm/PID/pid.c
    ${PLATFORM_RUNTIME_DIR}/module/algorithm/VMC/VMC_calc.c
    ${PLATFORM_RUNTIME_DIR}/module/algorithm/kalman/kalman_filter.c
    ${PLATFORM_RUNTIME_DIR}/module/algorithm/mahony/mahony_filter.c
    ${PLATFORM_RUNTIME_DIR}/module/controller/controller.c
    ${PLATFORM_RUNTIME_DIR}/module/lib/user_lib.c
    ${PLATFORM_RUNTIME_DIR}/module/message_center/message_center.c
  )

  set(BALANCE_CHASSIS_CMSIS_DSP_SOURCES
    ${CHASSIS_CMSIS_DIR}/DSP/Source/BasicMathFunctions/BasicMathFunctions.c
    ${CHASSIS_CMSIS_DIR}/DSP/Source/CommonTables/CommonTables.c
    ${CHASSIS_CMSIS_DIR}/DSP/Source/ComplexMathFunctions/ComplexMathFunctions.c
    ${CHASSIS_CMSIS_DIR}/DSP/Source/ControllerFunctions/ControllerFunctions.c
    ${CHASSIS_CMSIS_DIR}/DSP/Source/FastMathFunctions/FastMathFunctions.c
    ${CHASSIS_CMSIS_DIR}/DSP/Source/FilteringFunctions/FilteringFunctions.c
    ${CHASSIS_CMSIS_DIR}/DSP/Source/MatrixFunctions/MatrixFunctions.c
    ${CHASSIS_CMSIS_DIR}/DSP/Source/StatisticsFunctions/StatisticsFunctions.c
    ${CHASSIS_CMSIS_DIR}/DSP/Source/SupportFunctions/SupportFunctions.c
    ${CHASSIS_CMSIS_DIR}/DSP/Source/TransformFunctions/TransformFunctions.c
  )

  add_library(cmsis_dsp_sitl STATIC
    ${BALANCE_CHASSIS_CMSIS_DSP_SOURCES}
  )

  target_include_directories(cmsis_dsp_sitl
    PRIVATE
      ${PLATFORM_SITL_BSP_DIR}
      ${CHASSIS_HAL_DIR}/Inc
      ${CHASSIS_CMSIS_DIR}/Device/ST/STM32H7xx/Include
      ${CHASSIS_CMSIS_DIR}/Include
      ${CHASSIS_CMSIS_DIR}/DSP/Include
  )

  target_compile_definitions(cmsis_dsp_sitl
    PRIVATE
      USE_HAL_DRIVER
      STM32H723xx
      ARM_MATH_CM7
  )

  target_compile_options(cmsis_dsp_sitl
    PRIVATE
      -Wall -Wextra -Wno-unused-parameter -O0 -g3
  )

  add_executable(balance_chassis_sitl
    ${PLATFORM_SITL_BSP_DIR}/main_sitl.c
    ${PLATFORM_LEGACY_APP_DIR}/app_bringup/freertos_app.c
    ${BALANCE_CHASSIS_FREERTOS_SOURCES}
    ${BALANCE_CHASSIS_SITL_BSP_SOURCES}
    ${BALANCE_CHASSIS_LEGACY_APP_SOURCES}
  )

  target_include_directories(balance_chassis_sitl
    PRIVATE
      # SITL overrides (must be first to override main.h, etc)
      ${PLATFORM_SITL_BSP_DIR}
      # Original hardware headers so app/module can find them
      ${PLATFORM_BOARD_BSP_DIR}
      ${PLATFORM_DM_MOTOR_DIR}
      ${PLATFORM_BMI088_DIR}
      ${PLATFORM_REMOTE_CONTROL_DIR}
      ${PLATFORM_LEGACY_APP_DIR}
      ${PLATFORM_CONTROL_CONTRACT_DIR}
      ${PLATFORM_RUNTIME_SERVICE_DIR}
      # CMSIS / Core headers needed by FreeRTOS CMSIS wrapper and others
      ${CHASSIS_HAL_DIR}/Inc
      ${CHASSIS_CMSIS_DIR}/Device/ST/STM32H7xx/Include
      ${CHASSIS_CMSIS_DIR}/Include
      ${CHASSIS_CMSIS_DIR}/DSP/Include
      ${GENERATED_RAW_DIR}/Inc
      # FreeRTOS headers
      ${CHASSIS_FREERTOS_DIR}/include
      ${CHASSIS_FREERTOS_DIR}/CMSIS_RTOS
      ${PLATFORM_FREERTOS_PORT_DIR}
      # Module headers
      ${PLATFORM_RUNTIME_DIR}/module/algorithm/EKF
      ${PLATFORM_RUNTIME_DIR}/module/algorithm/PID
      ${PLATFORM_RUNTIME_DIR}/module/algorithm/VMC
      ${PLATFORM_RUNTIME_DIR}/module/algorithm/kalman
      ${PLATFORM_RUNTIME_DIR}/module/algorithm/mahony
      ${PLATFORM_RUNTIME_DIR}/module/controller
      ${PLATFORM_RUNTIME_DIR}/module/lib
      ${PLATFORM_RUNTIME_DIR}/module/message_center
  )

  target_compile_definitions(balance_chassis_sitl
    PRIVATE
      USE_HAL_DRIVER
      STM32H723xx
      SITL_BUILD
      ARM_MATH_CM7
  )

  if(PLATFORM_SIM_ASAN)
    target_compile_options(balance_chassis_sitl
      PRIVATE
        -fsanitize=address
        -fno-omit-frame-pointer
    )
    target_link_options(balance_chassis_sitl
      PRIVATE
        -fsanitize=address
    )
  endif()

  target_link_libraries(balance_chassis_sitl
    PRIVATE
      cmsis_dsp_sitl
      balance_chassis_device
      pthread
      m
  )
endif()

if((PLATFORM_TARGET_HW OR PLATFORM_TARGET_SIM) AND NOT TARGET balance_chassis_device)
  add_library(balance_chassis_device STATIC
    ${BALANCE_CHASSIS_DEVICE_SOURCES}
  )

  target_include_directories(balance_chassis_device
    PUBLIC
      ${PLATFORM_DEVICE_DIR}
      ${PLATFORM_DEVICE_DIR}/imu
      ${PLATFORM_DEVICE_DIR}/remote
      ${PLATFORM_DEVICE_DIR}/actuator
      ${PLATFORM_DEVICE_DIR}/actuator/motor
      ${PLATFORM_CONTROL_CONTRACT_DIR}
  )

  target_compile_options(balance_chassis_device
    PRIVATE
      -Wall
      -Wextra
      -Wno-unused-parameter
  )

  if(PLATFORM_TARGET_HW)
    target_include_directories(balance_chassis_device
      PUBLIC
        ${PLATFORM_BOARD_BSP_DIR}
        ${PLATFORM_REMOTE_CONTROL_DIR}
        ${PLATFORM_DM_MOTOR_DIR}
        ${PLATFORM_BMI088_DIR}
        ${GENERATED_RAW_DIR}/Inc
        ${CHASSIS_HAL_DIR}/Inc
        ${CHASSIS_CMSIS_DIR}/Device/ST/STM32H7xx/Include
        ${CHASSIS_CMSIS_DIR}/Include
        ${CHASSIS_CMSIS_DIR}/DSP/Include
    )
    target_link_libraries(balance_chassis_device
      PUBLIC
        balance_chassis_generated_raw
        balance_chassis_bsp_seed
    )
    target_compile_definitions(balance_chassis_device
      PUBLIC
        USE_HAL_DRIVER
        STM32H723xx
        PLATFORM_DEVICE_BACKEND_HW
    )
    target_compile_options(balance_chassis_device
      PRIVATE
        ${BALANCE_CHASSIS_COMMON_FLAGS}
    )
  endif()

  if(PLATFORM_TARGET_SIM)
    target_compile_definitions(balance_chassis_device
      PUBLIC
        PLATFORM_DEVICE_BACKEND_SITL
        SITL_BUILD
    )
  endif()
endif()
