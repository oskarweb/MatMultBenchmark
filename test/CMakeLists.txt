include(${CMAKE_SOURCE_DIR}/cmake/utils.cmake)
include(FetchContent)

FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/heads/main.zip
)

if (WIN32)
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
endif()

FetchContent_MakeAvailable(googletest)

set(TESTS_TARGET_NAME ocldemolib_tests)

set(BOOST_COMPONENTS program_options)

add_executable(${TESTS_TARGET_NAME})

add_subdirectory(fixtures)
add_subdirectory(tests)

target_sources(ocldemolib_tests
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/tests_main.cpp
)

link_boost(${TESTS_TARGET_NAME} ${BOOST_COMPONENTS})

target_link_libraries(${TESTS_TARGET_NAME}
    PRIVATE OclDemoLib
    PRIVATE gtest
    PRIVATE OpenCL::OpenCL
)

include(GoogleTest)
gtest_discover_tests(${TESTS_TARGET_NAME})

set(ALL_TARGETS ${ALL_TARGETS} ${TESTS_TARGET_NAME} CACHE INTERNAL "All targets")