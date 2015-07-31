cmake_minimum_required(VERSION 2.8)

set(LLVM_CONFIG "llvm-config" CACHE STRING "default llvm-config command")

set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} $ENV{CMAKE_MODULE_PATH})
set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} $ENV{CMAKE_PREFIX_PATH})
set(CMAKE_C_COMPILER $ENV{CC})
set(CMAKE_CXX_COMPILER $ENV{CXX})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
if (${LLVM_ENABLE_RTTI})
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -frtti")
else (${LLVM_ENABLE_RTTI})
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti")
endif (${LLVM_ENABLE_RTTI})

### should work, but actually it is not on current Debian with llvm-3.5
#find_package(LLVM REQUIRED CONFIG)
execute_process(COMMAND ${LLVM_CONFIG} "--cflags" RESULT_VARIABLE LLVM_CONFIG_RES OUTPUT_VARIABLE LLVM_DEFINITIONS)
if (NOT LLVM_CONFIG_RES EQUAL 0)
  message (FATAL_ERROR "command ${LLVM_CONFIG} not found in PATH : error ${LLVM_CONFIG_RES}")
endif ()
                
add_definitions(${LLVM_DEFINITIONS})
include_directories(${LLVM_INCLUDE_DIRS})

add_subdirectory(CFI)
