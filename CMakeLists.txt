cmake_minimum_required(VERSION 3.10)

set(PROJECT_NAME "OclDemo")

set(ALL_TARGETS ${PROJECT_NAME} CACHE INTERNAL "All targets")

if(WIN32)
    set(HOME "$ENV{USERPROFILE}")
else()
    set(HOME "$ENV{HOME}")
endif()

project(${PROJECT_NAME} VERSION 1.0)

message(STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID}")

set(STACK_SIZE "1000000" CACHE STRING "Stack size in bytes")

message(STATUS "Stack size: ${STACK_SIZE} bytes")

if(MSVC)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /STACK:${STACK_SIZE}")
    add_compile_options(/W4 /WX /permissive-)
else()
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--stack,${STACK_SIZE}")
    add_compile_options(-Wall -Wextra -Wpedantic -Werror)
endif()

set(CMAKE_PREFIX_PATH "${HOME}/OpenCL_SDK")

set(THIRD_PARTY_DIR "${CMAKE_SOURCE_DIR}/third_party")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

find_package(OpenCL REQUIRED)

set(SRC_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/src")

add_subdirectory(${SRC_DIRECTORY})

add_executable(${PROJECT_NAME} ${SRC_DIRECTORY}/main.cpp)

target_link_libraries(${PROJECT_NAME} 
    PRIVATE OpenCL::OpenCL
    PRIVATE OclDemo::Lib
)

foreach(TARGET IN LISTS ALL_TARGETS)
    target_compile_definitions(${TARGET} PRIVATE STACK_SIZE=${STACK_SIZE})
endforeach()
