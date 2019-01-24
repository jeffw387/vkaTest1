cmake_minimum_required(VERSION 3.10)
project(vkaTest1 LANGUAGES CXX)

include(${CMAKE_CURRENT_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

add_executable(vkaTest1Main src/main.cpp)
target_link_libraries(vkaTest1Main PRIVATE ${CONAN_LIBS})

add_executable(triangle src/triangle.cpp)
target_link_libraries(triangle PRIVATE ${CONAN_LIBS})

add_executable(catch_tests src/catch_main.cpp src/monotonic_allocator.test.cpp)
target_link_libraries(catch_tests PRIVATE ${CONAN_LIBS})