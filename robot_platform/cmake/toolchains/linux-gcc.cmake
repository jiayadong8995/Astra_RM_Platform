set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR x86_64)

set(CMAKE_C_COMPILER gcc)
set(CMAKE_CXX_COMPILER g++)
set(CMAKE_ASM_COMPILER gcc)

set(CMAKE_C_FLAGS "-Wall -Wextra -D__LINUX__ -O0 -g3" CACHE INTERNAL "C Compiler options")
set(CMAKE_CXX_FLAGS "-Wall -Wextra -D__LINUX__ -O0 -g3" CACHE INTERNAL "C++ Compiler options")

# Add pthread for FreeRTOS Linux port
set(CMAKE_EXE_LINKER_FLAGS "-pthread" CACHE INTERNAL "Executable Linker options")
