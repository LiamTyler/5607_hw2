cmake_minimum_required(VERSION 2.8.12)
project(RayTracer)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")

add_subdirectory(ext/nanogui)
include_directories(ext/nanogui/include)
include_directories(${NANOGUI_EXTRA_INCS})
add_definitions(${NANOGUI_EXTRA_DEFS})

file(GLOB SRC_FILES src/*.cpp)
include_directories(ext/)
include_directories(src/)
add_executable(RayTracer ${SRC_FILES})
target_link_libraries(RayTracer nanogui ${NANOGUI_EXTRA_LIBS})
