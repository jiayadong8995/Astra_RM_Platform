cmake_minimum_required(VERSION 3.22)

project(robot_platform C ASM)

# Export compilation database so editor tooling can resolve includes,
# macros, and per-file flags for jump-to-definition.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(PLATFORM_TARGET_HW "Build hardware target" ON)
option(PLATFORM_TARGET_REPLAY "Build replay target" OFF)
option(PLATFORM_TARGET_SIM "Build simulation target" OFF)
option(PLATFORM_HOST_TESTS "Build Linux host-side C tests" OFF)
option(PLATFORM_HOST_TEST_SANITIZERS "Enable sanitizers for Linux host-side C tests" ON)
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
message(STATUS "PLATFORM_HOST_TESTS=${PLATFORM_HOST_TESTS}")
message(STATUS "PLATFORM_HOST_TEST_SANITIZERS=${PLATFORM_HOST_TEST_SANITIZERS}")
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
set(PLATFORM_BSP_DRIVERS_DIR ${PLATFORM_RUNTIME_DIR}/bsp/drivers)
set(PLATFORM_DRIVERS_REMOTE_DIR ${PLATFORM_BSP_DRIVERS_DIR}/remote/dbus)
set(PLATFORM_DRIVERS_MOTOR_DIR ${PLATFORM_BSP_DRIVERS_DIR}/actuator/motor/dm4310)
set(PLATFORM_DRIVERS_IMU_DIR ${PLATFORM_BSP_DRIVERS_DIR}/imu/bmi088)
set(PLATFORM_LEGACY_APP_DIR ${PLATFORM_RUNTIME_DIR}/app/balance_chassis)
set(PLATFORM_APP_STARTUP_DIR ${PLATFORM_LEGACY_APP_DIR}/app_startup)
set(PLATFORM_APP_IO_DIR ${PLATFORM_LEGACY_APP_DIR}/app_io)
set(PLATFORM_CONTROL_CONTRACT_DIR ${PLATFORM_RUNTIME_DIR}/control/contracts)
set(PLATFORM_MODULE_CONTROL_LIB_DIR ${PLATFORM_RUNTIME_DIR}/module/lib/control)
set(PLATFORM_HOST_TEST_DIR ${PLATFORM_RUNTIME_DIR}/tests/host)

set(BALANCE_CHASSIS_APP_SOURCES
  ${PLATFORM_APP_STARTUP_DIR}/balance_chassis_app_startup.c
  ${PLATFORM_LEGACY_APP_DIR}/app_bringup/chassis_task.c
  ${PLATFORM_LEGACY_APP_DIR}/app_bringup/task_registry.c
  ${PLATFORM_LEGACY_APP_DIR}/app_bringup/remote_task.c
  ${PLATFORM_LEGACY_APP_DIR}/app_intent/remote_intent.c
  ${PLATFORM_LEGACY_APP_DIR}/app_io/chassis_topics.c
  ${PLATFORM_RUNTIME_DIR}/control/state/ins_state_estimator.c
  ${PLATFORM_RUNTIME_DIR}/control/state/ins_task.c
  ${PLATFORM_RUNTIME_DIR}/control/state/chassis_observer.c
  ${PLATFORM_RUNTIME_DIR}/control/state/observe_task.c
  ${PLATFORM_RUNTIME_DIR}/control/task_registry/control_task_registry.c
  ${PLATFORM_RUNTIME_DIR}/control/controllers/balance_controller.c
  ${PLATFORM_RUNTIME_DIR}/control/controllers/chassis_control_task.c
  ${PLATFORM_RUNTIME_DIR}/control/constraints/actuator_constraints.c
  ${PLATFORM_RUNTIME_DIR}/control/readiness.c
  ${PLATFORM_RUNTIME_DIR}/control/execution/actuator_gateway.c
  ${PLATFORM_RUNTIME_DIR}/control/execution/motor_control_task.c
  ${PLATFORM_MODULE_CONTROL_LIB_DIR}/control_math.c
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

if(PLATFORM_HOST_TESTS)
  enable_testing()
  set(PLATFORM_SITL_BSP_DIR ${PLATFORM_RUNTIME_DIR}/bsp/sitl)

  set(PLATFORM_MESSAGE_CENTER_HOST_TEST_SOURCE
    ${PLATFORM_HOST_TEST_DIR}/test_message_center.c
  )
  if(NOT EXISTS ${PLATFORM_MESSAGE_CENTER_HOST_TEST_SOURCE})
    set(PLATFORM_MESSAGE_CENTER_HOST_TEST_SOURCE
      ${CMAKE_CURRENT_BINARY_DIR}/generated_host_tests/test_message_center_bootstrap.c
    )
    file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/generated_host_tests)
    file(WRITE ${PLATFORM_MESSAGE_CENTER_HOST_TEST_SOURCE}
      "#include <stdint.h>\n"
      "int main(void)\n"
      "{\n"
      "    return 0;\n"
      "}\n"
    )
  endif()

  add_executable(test_message_center
    ${PLATFORM_RUNTIME_DIR}/module/message_center/message_center.c
    ${PLATFORM_MESSAGE_CENTER_HOST_TEST_SOURCE}
  )

  target_include_directories(test_message_center
    PRIVATE
      ${PLATFORM_RUNTIME_DIR}/module/message_center
      ${PLATFORM_CONTROL_CONTRACT_DIR}
      ${PLATFORM_HOST_TEST_DIR}
  )

  target_compile_features(test_message_center PRIVATE c_std_11)

  target_compile_options(test_message_center
    PRIVATE
      -Wall
      -Wextra
      -Wpedantic
      -Wno-unused-parameter
      -O0
      -g3
  )

  if(PLATFORM_HOST_TEST_SANITIZERS AND CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(test_message_center
      PRIVATE
        -fsanitize=address,undefined
        -fno-omit-frame-pointer
    )
    target_link_options(test_message_center
      PRIVATE
        -fsanitize=address,undefined
    )
  endif()

  add_test(NAME test_message_center COMMAND test_message_center)
  set_tests_properties(test_message_center PROPERTIES
    ENVIRONMENT "ASAN_OPTIONS=detect_leaks=0"
  )

  add_executable(test_actuator_gateway
    ${PLATFORM_RUNTIME_DIR}/control/execution/actuator_gateway.c
    ${PLATFORM_HOST_TEST_DIR}/test_actuator_gateway.c
    ${PLATFORM_RUNTIME_DIR}/bsp/ports_fake.c
  )

  target_include_directories(test_actuator_gateway
    PRIVATE
      ${PLATFORM_RUNTIME_DIR}/control/execution
      ${PLATFORM_RUNTIME_DIR}/bsp
      ${PLATFORM_CONTROL_CONTRACT_DIR}
      ${PLATFORM_HOST_TEST_DIR}
  )

  target_compile_features(test_actuator_gateway PRIVATE c_std_11)

  target_compile_options(test_actuator_gateway
    PRIVATE
      -Wall
      -Wextra
      -Wpedantic
      -Wno-unused-parameter
      -O0
      -g3
  )

  if(PLATFORM_HOST_TEST_SANITIZERS AND CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(test_actuator_gateway
      PRIVATE
        -fsanitize=address,undefined
        -fno-omit-frame-pointer
    )
    target_link_options(test_actuator_gateway
      PRIVATE
        -fsanitize=address,undefined
    )
  endif()

  add_test(NAME test_actuator_gateway COMMAND test_actuator_gateway)
  set_tests_properties(test_actuator_gateway PROPERTIES
    ENVIRONMENT "ASAN_OPTIONS=detect_leaks=0"
  )

  set(BALANCE_SAFETY_HOST_BSP_SOURCES
    ${PLATFORM_SITL_BSP_DIR}/hal_stub.c
    ${PLATFORM_SITL_BSP_DIR}/bsp_dwt_sitl.c
    ${PLATFORM_SITL_BSP_DIR}/bsp_PWM_sitl.c
    ${PLATFORM_SITL_BSP_DIR}/can_bsp_sitl.c
    ${PLATFORM_SITL_BSP_DIR}/BMI088driver_sitl.c
    ${PLATFORM_SITL_BSP_DIR}/remote_control_sitl.c
    ${PLATFORM_SITL_BSP_DIR}/dm4310_drv_sitl.c
  )

  set(BALANCE_SAFETY_HOST_FAKE_PORT_SOURCE
    ${PLATFORM_RUNTIME_DIR}/bsp/ports_fake.c
  )

  set(BALANCE_SAFETY_HOST_RUNTIME_SOURCES
    ${PLATFORM_LEGACY_APP_DIR}/app_bringup/remote_task.c
    ${PLATFORM_LEGACY_APP_DIR}/app_intent/remote_intent.c
    ${PLATFORM_LEGACY_APP_DIR}/app_io/chassis_topics.c
    ${PLATFORM_RUNTIME_DIR}/control/state/chassis_observer.c
    ${PLATFORM_RUNTIME_DIR}/control/state/ins_state_estimator.c
    ${PLATFORM_RUNTIME_DIR}/control/state/observe_task.c
    ${PLATFORM_RUNTIME_DIR}/control/controllers/balance_controller.c
    ${PLATFORM_RUNTIME_DIR}/control/controllers/chassis_control_task.c
    ${PLATFORM_RUNTIME_DIR}/control/readiness.c
    ${PLATFORM_RUNTIME_DIR}/control/constraints/actuator_constraints.c
    ${PLATFORM_RUNTIME_DIR}/control/execution/actuator_gateway.c
    ${PLATFORM_RUNTIME_DIR}/control/execution/motor_control_task.c
    ${PLATFORM_MODULE_CONTROL_LIB_DIR}/control_math.c
    ${PLATFORM_RUNTIME_DIR}/module/algorithm/EKF/QuaternionEKF.c
    ${PLATFORM_RUNTIME_DIR}/module/algorithm/PID/pid.c
    ${PLATFORM_RUNTIME_DIR}/module/algorithm/VMC/VMC_calc.c
    ${PLATFORM_RUNTIME_DIR}/module/algorithm/kalman/kalman_filter.c
    ${PLATFORM_RUNTIME_DIR}/module/algorithm/mahony/mahony_filter.c
    ${PLATFORM_RUNTIME_DIR}/module/controller/controller.c
    ${PLATFORM_RUNTIME_DIR}/module/lib/user_lib.c
    ${PLATFORM_RUNTIME_DIR}/module/message_center/message_center.c
    ${PLATFORM_HOST_TEST_DIR}/test_support/host_os_stubs.c
    ${PLATFORM_HOST_TEST_DIR}/test_support/balance_safety_harness.c
    ${BALANCE_SAFETY_HOST_BSP_SOURCES}
    ${BALANCE_SAFETY_HOST_FAKE_PORT_SOURCE}
  )

  add_library(cmsis_dsp_host STATIC
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

  target_include_directories(cmsis_dsp_host
    PRIVATE
      ${PLATFORM_HOST_TEST_DIR}/test_support/os
      ${PLATFORM_SITL_BSP_DIR}
      ${CHASSIS_HAL_DIR}/Inc
      ${CHASSIS_CMSIS_DIR}/Device/ST/STM32H7xx/Include
      ${CHASSIS_CMSIS_DIR}/Include
      ${CHASSIS_CMSIS_DIR}/DSP/Include
  )

  target_compile_definitions(cmsis_dsp_host
    PRIVATE
      USE_HAL_DRIVER
      STM32H723xx
      SITL_BUILD
      ARM_MATH_CM7
  )

  target_compile_options(cmsis_dsp_host
    PRIVATE
      -Wall
      -Wextra
      -Wpedantic
      -Wno-unused-parameter
      -O0
      -g3
  )

  add_library(balance_safety_host_runtime STATIC
    ${BALANCE_SAFETY_HOST_RUNTIME_SOURCES}
  )

  target_include_directories(balance_safety_host_runtime
    PUBLIC
      ${PLATFORM_HOST_TEST_DIR}/test_support/os
      ${PLATFORM_SITL_BSP_DIR}
      ${PLATFORM_BOARD_BSP_DIR}
      ${PLATFORM_RUNTIME_DIR}/bsp
      ${PLATFORM_BSP_DRIVERS_DIR}
      ${PLATFORM_BSP_DRIVERS_DIR}/imu
      ${PLATFORM_BSP_DRIVERS_DIR}/remote
      ${PLATFORM_BSP_DRIVERS_DIR}/actuator
      ${PLATFORM_BSP_DRIVERS_DIR}/actuator/motor
      ${PLATFORM_DRIVERS_REMOTE_DIR}
      ${PLATFORM_DRIVERS_MOTOR_DIR}
      ${PLATFORM_DRIVERS_IMU_DIR}
      ${PLATFORM_LEGACY_APP_DIR}
      ${PLATFORM_APP_IO_DIR}
      ${PLATFORM_CONTROL_CONTRACT_DIR}
      ${CHASSIS_HAL_DIR}/Inc
      ${CHASSIS_CMSIS_DIR}/Device/ST/STM32H7xx/Include
      ${CHASSIS_CMSIS_DIR}/Include
      ${CHASSIS_CMSIS_DIR}/DSP/Include
      ${GENERATED_RAW_DIR}/Inc
      ${CHASSIS_FREERTOS_DIR}/include
      ${CHASSIS_FREERTOS_DIR}/CMSIS_RTOS
      ${PLATFORM_FREERTOS_PORT_DIR}
      ${PLATFORM_RUNTIME_DIR}/control/controllers
      ${PLATFORM_RUNTIME_DIR}/control/state
      ${PLATFORM_RUNTIME_DIR}/control/execution
      ${PLATFORM_RUNTIME_DIR}/control/constraints
      ${PLATFORM_RUNTIME_DIR}/control/internal
      ${PLATFORM_RUNTIME_DIR}/module/algorithm/EKF
      ${PLATFORM_RUNTIME_DIR}/module/algorithm/PID
      ${PLATFORM_RUNTIME_DIR}/module/algorithm/VMC
      ${PLATFORM_RUNTIME_DIR}/module/algorithm/kalman
      ${PLATFORM_RUNTIME_DIR}/module/algorithm/mahony
      ${PLATFORM_RUNTIME_DIR}/module/controller
      ${PLATFORM_RUNTIME_DIR}/module/lib
      ${PLATFORM_MODULE_CONTROL_LIB_DIR}
      ${PLATFORM_RUNTIME_DIR}/module/message_center
      ${PLATFORM_HOST_TEST_DIR}/test_support
      ${PLATFORM_HOST_TEST_DIR}
  )

  target_compile_definitions(balance_safety_host_runtime
    PUBLIC
      USE_HAL_DRIVER
      STM32H723xx
      SITL_BUILD
      ARM_MATH_CM7
  )

  target_compile_features(balance_safety_host_runtime PUBLIC c_std_11)

  target_compile_options(balance_safety_host_runtime
    PRIVATE
      -Wall
      -Wextra
      -Wpedantic
      -Wno-unused-parameter
      -O0
      -g3
  )

  if(PLATFORM_HOST_TEST_SANITIZERS AND CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(balance_safety_host_runtime
      PRIVATE
        -fsanitize=address,undefined
        -fno-omit-frame-pointer
    )
    target_link_options(balance_safety_host_runtime
      PRIVATE
        -fsanitize=address,undefined
    )
  endif()

  target_link_libraries(balance_safety_host_runtime
    PUBLIC
      cmsis_dsp_host
      pthread
      m
  )

  add_executable(test_balance_safety_path
    ${PLATFORM_HOST_TEST_DIR}/test_balance_safety_path.c
  )
  target_link_libraries(test_balance_safety_path PRIVATE balance_safety_host_runtime)
  target_compile_features(test_balance_safety_path PRIVATE c_std_11)
  if(PLATFORM_HOST_TEST_SANITIZERS AND CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(test_balance_safety_path PRIVATE -fsanitize=address,undefined -fno-omit-frame-pointer)
    target_link_options(test_balance_safety_path PRIVATE -fsanitize=address,undefined)
  endif()
  add_test(NAME test_balance_safety_path COMMAND test_balance_safety_path)
  set_tests_properties(test_balance_safety_path PROPERTIES
    ENVIRONMENT "ASAN_OPTIONS=detect_leaks=0"
  )

  add_executable(test_balance_app_startup
    ${PLATFORM_APP_STARTUP_DIR}/balance_chassis_app_startup.c
    ${PLATFORM_HOST_TEST_DIR}/test_balance_app_startup.c
  )
  target_include_directories(test_balance_app_startup
    PRIVATE
      ${PLATFORM_LEGACY_APP_DIR}
      ${PLATFORM_APP_STARTUP_DIR}
      ${PLATFORM_HOST_TEST_DIR}
  )
  target_compile_features(test_balance_app_startup PRIVATE c_std_11)
  target_compile_options(test_balance_app_startup
    PRIVATE
      -Wall
      -Wextra
      -Wpedantic
      -Wno-unused-parameter
      -O0
      -g3
  )
  if(PLATFORM_HOST_TEST_SANITIZERS AND CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(test_balance_app_startup PRIVATE -fsanitize=address,undefined -fno-omit-frame-pointer)
    target_link_options(test_balance_app_startup PRIVATE -fsanitize=address,undefined)
  endif()
  add_test(NAME test_balance_app_startup COMMAND test_balance_app_startup)
  set_tests_properties(test_balance_app_startup PROPERTIES
    ENVIRONMENT "ASAN_OPTIONS=detect_leaks=0"
  )

  add_executable(test_safety_mapping
    ${PLATFORM_HOST_TEST_DIR}/test_safety_mapping.c
  )
  target_link_libraries(test_safety_mapping PRIVATE balance_safety_host_runtime)
  target_compile_features(test_safety_mapping PRIVATE c_std_11)
  if(PLATFORM_HOST_TEST_SANITIZERS AND CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(test_safety_mapping PRIVATE -fsanitize=address,undefined -fno-omit-frame-pointer)
    target_link_options(test_safety_mapping PRIVATE -fsanitize=address,undefined)
  endif()
  add_test(NAME test_safety_mapping COMMAND test_safety_mapping)
  set_tests_properties(test_safety_mapping PROPERTIES
    ENVIRONMENT "ASAN_OPTIONS=detect_leaks=0"
  )

  add_executable(test_safety_sensor_faults
    ${PLATFORM_HOST_TEST_DIR}/test_safety_sensor_faults.c
  )
  target_link_libraries(test_safety_sensor_faults PRIVATE balance_safety_host_runtime)
  target_compile_features(test_safety_sensor_faults PRIVATE c_std_11)
  if(PLATFORM_HOST_TEST_SANITIZERS AND CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(test_safety_sensor_faults PRIVATE -fsanitize=address,undefined -fno-omit-frame-pointer)
    target_link_options(test_safety_sensor_faults PRIVATE -fsanitize=address,undefined)
  endif()
  add_test(NAME test_safety_sensor_faults COMMAND test_safety_sensor_faults)
  set_tests_properties(test_safety_sensor_faults PROPERTIES
    ENVIRONMENT "ASAN_OPTIONS=detect_leaks=0"
  )

  add_executable(test_safety_arming
    ${PLATFORM_HOST_TEST_DIR}/test_safety_arming.c
  )
  target_link_libraries(test_safety_arming PRIVATE balance_safety_host_runtime)
  target_compile_features(test_safety_arming PRIVATE c_std_11)
  if(PLATFORM_HOST_TEST_SANITIZERS AND CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(test_safety_arming PRIVATE -fsanitize=address,undefined -fno-omit-frame-pointer)
    target_link_options(test_safety_arming PRIVATE -fsanitize=address,undefined)
  endif()
  add_test(NAME test_safety_arming COMMAND test_safety_arming)
  set_tests_properties(test_safety_arming PROPERTIES
    ENVIRONMENT "ASAN_OPTIONS=detect_leaks=0"
  )

  add_executable(test_safety_saturation
    ${PLATFORM_HOST_TEST_DIR}/test_safety_saturation.c
  )
  target_link_libraries(test_safety_saturation PRIVATE balance_safety_host_runtime)
  target_compile_features(test_safety_saturation PRIVATE c_std_11)
  if(PLATFORM_HOST_TEST_SANITIZERS AND CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(test_safety_saturation PRIVATE -fsanitize=address,undefined -fno-omit-frame-pointer)
    target_link_options(test_safety_saturation PRIVATE -fsanitize=address,undefined)
  endif()
  add_test(NAME test_safety_saturation COMMAND test_safety_saturation)
  set_tests_properties(test_safety_saturation PROPERTIES
    ENVIRONMENT "ASAN_OPTIONS=detect_leaks=0"
  )

  add_executable(test_safety_wheel_leg
    ${PLATFORM_HOST_TEST_DIR}/test_safety_wheel_leg.c
  )
  target_link_libraries(test_safety_wheel_leg PRIVATE balance_safety_host_runtime)
  target_compile_features(test_safety_wheel_leg PRIVATE c_std_11)
  if(PLATFORM_HOST_TEST_SANITIZERS AND CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(test_safety_wheel_leg PRIVATE -fsanitize=address,undefined -fno-omit-frame-pointer)
    target_link_options(test_safety_wheel_leg PRIVATE -fsanitize=address,undefined)
  endif()
  add_test(NAME test_safety_wheel_leg COMMAND test_safety_wheel_leg)
  set_tests_properties(test_safety_wheel_leg PROPERTIES
    ENVIRONMENT "ASAN_OPTIONS=detect_leaks=0"
  )

  add_executable(test_bsp_ports_compile
    ${PLATFORM_HOST_TEST_DIR}/test_bsp_ports_compile.c
  )
  target_link_libraries(test_bsp_ports_compile PRIVATE balance_safety_host_runtime)
  target_compile_features(test_bsp_ports_compile PRIVATE c_std_11)
  if(PLATFORM_HOST_TEST_SANITIZERS AND CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(test_bsp_ports_compile PRIVATE -fsanitize=address,undefined -fno-omit-frame-pointer)
    target_link_options(test_bsp_ports_compile PRIVATE -fsanitize=address,undefined)
  endif()
  add_test(NAME test_bsp_ports_compile COMMAND test_bsp_ports_compile)
  set_tests_properties(test_bsp_ports_compile PROPERTIES
    ENVIRONMENT "ASAN_OPTIONS=detect_leaks=0"
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
  ${PLATFORM_BOARD_BSP_DIR}/bsp_ports_hw.c
  ${PLATFORM_DRIVERS_REMOTE_DIR}/remote_control.c
  ${PLATFORM_DRIVERS_MOTOR_DIR}/dm4310_drv.c
  ${PLATFORM_BSP_DRIVERS_DIR}/imu/bmi088_device_hw.c
  ${PLATFORM_BSP_DRIVERS_DIR}/remote/dbus_remote_device_hw.c
  ${PLATFORM_BSP_DRIVERS_DIR}/actuator/motor/motor_actuator_device_hw.c
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
    ${PLATFORM_DRIVERS_REMOTE_DIR}
    ${PLATFORM_DRIVERS_MOTOR_DIR}
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
    ${PLATFORM_RUNTIME_DIR}/bsp
    ${PLATFORM_DRIVERS_REMOTE_DIR}
    ${PLATFORM_DRIVERS_MOTOR_DIR}
    ${PLATFORM_CONTROL_CONTRACT_DIR}
    ${PLATFORM_BSP_DRIVERS_DIR}
    ${PLATFORM_BSP_DRIVERS_DIR}/imu
    ${PLATFORM_BSP_DRIVERS_DIR}/remote
    ${PLATFORM_BSP_DRIVERS_DIR}/actuator
    ${PLATFORM_BSP_DRIVERS_DIR}/actuator/motor
    ${PLATFORM_DRIVERS_IMU_DIR}
    ${PLATFORM_LEGACY_APP_DIR}
    ${PLATFORM_APP_IO_DIR}
    ${CHASSIS_FREERTOS_DIR}/include
    ${CHASSIS_FREERTOS_DIR}/CMSIS_RTOS
    ${PLATFORM_FREERTOS_PORT_DIR}
    ${PLATFORM_RUNTIME_DIR}/module/message_center
    ${PLATFORM_MODULE_CONTROL_LIB_DIR}
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

add_library(cmsis_dsp_hw STATIC
  ${BALANCE_CHASSIS_CMSIS_DSP_SOURCES}
)

target_include_directories(cmsis_dsp_hw
  PRIVATE
    ${CHASSIS_HAL_DIR}/Inc
    ${CHASSIS_CMSIS_DIR}/Device/ST/STM32H7xx/Include
    ${CHASSIS_CMSIS_DIR}/Include
    ${CHASSIS_CMSIS_DIR}/DSP/Include
)

target_compile_definitions(cmsis_dsp_hw
  PRIVATE
    USE_HAL_DRIVER
    STM32H723xx
    ARM_MATH_CM7
)

target_compile_options(cmsis_dsp_hw
  PRIVATE
    ${BALANCE_CHASSIS_COMMON_FLAGS}
    -Wall
    -Wextra
    -Wno-unused-parameter
)

add_executable(balance_chassis_hw_seed.elf
  ${GENERATED_RAW_DIR}/Src/main.c
  ${GENERATED_RAW_DIR}/Src/freertos.c
  ${CHASSIS_ROOT}/Drivers/CMSIS/Device/ST/STM32H7xx/Source/Templates/gcc/startup_stm32h723xx.s
  ${BALANCE_CHASSIS_FREERTOS_SOURCES}
  ${BALANCE_CHASSIS_APP_SOURCES}
)

target_link_libraries(balance_chassis_hw_seed.elf
  PRIVATE
    balance_chassis_generated_raw
    balance_chassis_bsp_seed
    cmsis_dsp_hw
    m
)

target_include_directories(balance_chassis_hw_seed.elf
  PRIVATE
    ${CHASSIS_FREERTOS_DIR}/include
    ${CHASSIS_FREERTOS_DIR}/CMSIS_RTOS
    ${PLATFORM_FREERTOS_PORT_DIR}
    ${PLATFORM_BOARD_BSP_DIR}
    ${PLATFORM_RUNTIME_DIR}/bsp
    ${PLATFORM_BSP_DRIVERS_DIR}
    ${PLATFORM_BSP_DRIVERS_DIR}/imu
    ${PLATFORM_BSP_DRIVERS_DIR}/remote
    ${PLATFORM_BSP_DRIVERS_DIR}/actuator
    ${PLATFORM_BSP_DRIVERS_DIR}/actuator/motor
    ${PLATFORM_DRIVERS_REMOTE_DIR}
    ${PLATFORM_DRIVERS_MOTOR_DIR}
    ${PLATFORM_DRIVERS_IMU_DIR}
    ${PLATFORM_LEGACY_APP_DIR}
    ${PLATFORM_APP_STARTUP_DIR}
    ${PLATFORM_APP_IO_DIR}
    ${PLATFORM_CONTROL_CONTRACT_DIR}
    ${PLATFORM_RUNTIME_DIR}/control/controllers
    ${PLATFORM_RUNTIME_DIR}/control/state
    ${PLATFORM_RUNTIME_DIR}/control/execution
    ${PLATFORM_RUNTIME_DIR}/control/constraints
    ${PLATFORM_RUNTIME_DIR}/control/internal
    ${PLATFORM_RUNTIME_DIR}/module/algorithm/EKF
    ${PLATFORM_RUNTIME_DIR}/module/algorithm/PID
    ${PLATFORM_RUNTIME_DIR}/module/algorithm/VMC
    ${PLATFORM_RUNTIME_DIR}/module/algorithm/kalman
    ${PLATFORM_RUNTIME_DIR}/module/algorithm/mahony
    ${PLATFORM_RUNTIME_DIR}/module/controller
    ${PLATFORM_RUNTIME_DIR}/module/lib
    ${PLATFORM_MODULE_CONTROL_LIB_DIR}
    ${PLATFORM_RUNTIME_DIR}/module/message_center
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
    ${PLATFORM_SITL_BSP_DIR}/remote_control_sitl.c
    ${PLATFORM_SITL_BSP_DIR}/dm4310_drv_sitl.c
    ${PLATFORM_SITL_BSP_DIR}/bsp_ports_sitl.c
    ${PLATFORM_BSP_DRIVERS_DIR}/imu/bmi088_device_sitl.c
    ${PLATFORM_BSP_DRIVERS_DIR}/remote/dbus_remote_device_sitl.c
    ${PLATFORM_BSP_DRIVERS_DIR}/actuator/motor/motor_actuator_device_sitl.c
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
    ${BALANCE_CHASSIS_FREERTOS_SOURCES}
    ${BALANCE_CHASSIS_SITL_BSP_SOURCES}
    ${BALANCE_CHASSIS_APP_SOURCES}
  )

  target_include_directories(balance_chassis_sitl
    PRIVATE
      # SITL overrides (must be first to override main.h, etc)
      ${PLATFORM_SITL_BSP_DIR}
      ${PLATFORM_RUNTIME_DIR}/bsp
      # Original hardware headers so app/module can find them
      ${PLATFORM_BOARD_BSP_DIR}
      ${PLATFORM_DRIVERS_MOTOR_DIR}
      ${PLATFORM_DRIVERS_IMU_DIR}
      ${PLATFORM_DRIVERS_REMOTE_DIR}
      ${PLATFORM_BSP_DRIVERS_DIR}
      ${PLATFORM_BSP_DRIVERS_DIR}/imu
      ${PLATFORM_BSP_DRIVERS_DIR}/remote
      ${PLATFORM_BSP_DRIVERS_DIR}/actuator
      ${PLATFORM_BSP_DRIVERS_DIR}/actuator/motor
      ${PLATFORM_LEGACY_APP_DIR}
      ${PLATFORM_APP_STARTUP_DIR}
      ${PLATFORM_APP_IO_DIR}
      ${PLATFORM_CONTROL_CONTRACT_DIR}
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
      ${PLATFORM_MODULE_CONTROL_LIB_DIR}
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
      pthread
      m
  )
endif()
