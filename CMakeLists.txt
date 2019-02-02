cmake_minimum_required(VERSION 3.10)
project(vkaTest1 LANGUAGES CXX)

# Download automatically, you can also just copy the conan.cmake file
if(NOT EXISTS "${CMAKE_BINARY_DIR}/conan.cmake")
   message(STATUS "Downloading conan.cmake from https://github.com/conan-io/cmake-conan")
   file(DOWNLOAD "https://github.com/conan-io/cmake-conan/raw/v0.13/conan.cmake"
                 "${CMAKE_BINARY_DIR}/conan.cmake")
endif()

include(${CMAKE_BINARY_DIR}/conan.cmake)

conan_cmake_run(CONANFILE conanfile.py
                BASIC_SETUP
                PROFILE default
                BUILD missing)

add_subdirectory(src/shaders)

add_executable(vkaTest1Main src/main.cpp)
target_link_libraries(vkaTest1Main PRIVATE ${CONAN_LIBS})

add_executable(triangle src/triangle.cpp)
target_link_libraries(triangle PRIVATE ${CONAN_LIBS})

add_executable(catch_tests src/catch_main.cpp src/monotonic_allocator.test.cpp)
target_link_libraries(catch_tests PRIVATE ${CONAN_LIBS})