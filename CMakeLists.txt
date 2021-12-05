cmake_minimum_required(VERSION 3.10)

project(ZenonAPICaching VERSION 0.01 LANGUAGES CXX)

### cmake properties

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

### subprojects and libraries

find_package(Threads REQUIRED)

# unit test library catch2
add_subdirectory("${PROJECT_SOURCE_DIR}/thirdparty/catch2" REQUIRED)

# path to catch2 provided module files for automatic test discovery
list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/thirdparty/catch2/extras")

### targets

file(GLOB SRC src/*.cpp)

add_executable(caching_tests "${PROJECT_SOURCE_DIR}/test/cache.cpp" ${SRC})

target_include_directories(caching_tests PUBLIC "${PROJECT_BINARY_DIR}")
target_include_directories(caching_tests PUBLIC "${PROJECT_SOURCE_DIR}/include")
target_include_directories(caching_tests PUBLIC "${PROJECT_SOURCE_DIR}/thirdparty")
target_include_directories(caching_tests PUBLIC "${PROJECT_SOURCE_DIR}/thirdparty/catch2/src")

target_link_libraries(caching_tests Catch2::Catch2WithMain Catch2::Catch2 Threads::Threads)

### cmake test discovery, only required if ctest is wanted

#include(CTest)
## provided by catch2 in /extras
#include(Catch)
#catch_discover_tests(caching_tests)
