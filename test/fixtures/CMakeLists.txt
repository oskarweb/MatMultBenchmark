set(OCL_DEMO_TESTS_FIXTURES 
    "${CMAKE_CURRENT_SOURCE_DIR}/fixture_with_random.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/test_utils.hpp"
)

target_sources(${TESTS_TARGET_NAME}
    PRIVATE
    ${OCL_DEMO_TESTS_FIXTURES}
)

target_include_directories(${TESTS_TARGET_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})