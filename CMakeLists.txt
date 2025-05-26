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

if(WIN32)
    set(CMAKE_PREFIX_PATH "${HOME}/OpenCL_SDK")
    
    set(STACK_SIZE "1000000" CACHE STRING "Stack size in bytes")
    message(STATUS "Stack size: ${STACK_SIZE} bytes")
    add_compile_options(/W4 /WX /permissive-)
    
    if(MSVC)
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /STACK:${STACK_SIZE}")
    else()
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--stack,${STACK_SIZE}")
    endif()
else()
    add_compile_options(-Wall -Wextra -Wpedantic -Werror)
endif()

if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
endif()


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

add_custom_command(
    TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/src/kernels
            ${CMAKE_BINARY_DIR}/kernels
    COMMENT "Copying kernels directory to build directory"
)

make_directory(${CMAKE_BINARY_DIR}/kernels/bin)