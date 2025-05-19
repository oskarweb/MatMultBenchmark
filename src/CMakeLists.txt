list(APPEND HEADERS
    "${CMAKE_CURRENT_SOURCE_DIR}/include/benchmarks.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/matrix.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/thread_pool.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/utils.hpp"
)

list(APPEND SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/thread_pool.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/utils.cpp"
)

set(LIB_NAME "OclDemoLib")

add_library(${LIB_NAME} ${HEADERS} ${SOURCES})
add_library(OclDemo::Lib ALIAS ${LIB_NAME})

target_include_directories(${LIB_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

set(ALL_TARGETS ${ALL_TARGETS} ${LIB_NAME} CACHE INTERNAL "All targets")