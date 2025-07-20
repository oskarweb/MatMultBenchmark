set(OCL_DEMO_TESTS_SRCS 
    "${CMAKE_CURRENT_SOURCE_DIR}/matrix_tests.cpp"
)

target_sources(${TESTS_TARGET_NAME}
    PRIVATE
    ${OCL_DEMO_TESTS_SRCS}
)

target_include_directories(${TESTS_TARGET_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})