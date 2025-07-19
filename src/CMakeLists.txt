file(GLOB HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp")
file(GLOB SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

set(LIB_NAME "OclDemoLib")

add_library(${LIB_NAME} ${HEADERS} ${SOURCES})
add_library(OclDemo::Lib ALIAS ${LIB_NAME})

set(BOOST_COMPONENTS json)

find_package(Boost REQUIRED COMPONENTS ${BOOST_COMPONENTS} CONFIG)

target_link_libraries(${LIB_NAME}
    PRIVATE OpenCL::OpenCL
)

foreach(component IN LISTS BOOST_COMPONENTS)
    target_link_libraries(${LIB_NAME} PRIVATE Boost::${component})
endforeach()

target_include_directories(${LIB_NAME} PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/include 
    ${Boost_INCLUDE_DIRS}
)

set(ALL_TARGETS ${ALL_TARGETS} ${LIB_NAME} CACHE INTERNAL "All targets")