cmake_minimum_required(VERSION 3.10)

set(PROJECT_NAME "OclDemo")

if(WIN32)
    set(HOME "$ENV{USERPROFILE}")
else()
    set(HOME "$ENV{HOME}")
endif()

project(${PROJECT_NAME} VERSION 1.0)

message(STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID}")

if(MSVC)
    add_compile_options(/W4 /WX /permissive-)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -Werror)
endif()

set(CMAKE_PREFIX_PATH "${HOME}/OpenCL_SDK")

set(THIRD_PARTY_DIR "${CMAKE_SOURCE_DIR}/third_party")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

find_package(OpenCL REQUIRED)

set(LIBS 
    OpenCL::OpenCL
)

set(SRC_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/src")

set(OCL_DEMO_SRCS 
    "${SRC_DIRECTORY}/benchmarks.h"
    "${SRC_DIRECTORY}/main.cpp"
    "${SRC_DIRECTORY}/matrix.h"
    "${SRC_DIRECTORY}/thread_pool.h"
    "${SRC_DIRECTORY}/thread_pool.cpp"
    "${SRC_DIRECTORY}/utils.h"
)

add_executable(${PROJECT_NAME} ${OCL_DEMO_SRCS})

target_link_libraries(${PROJECT_NAME} ${LIBS})