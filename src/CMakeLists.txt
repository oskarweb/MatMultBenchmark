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

add_library(OclDemoLib ${HEADERS} ${SOURCES})
add_library(OclDemo::Lib ALIAS OclDemoLib)

target_include_directories(OclDemoLib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)