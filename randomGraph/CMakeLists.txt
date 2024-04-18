
cmake_minimum_required(VERSION 3.10.0)

project(TasklibBenchmark LANGUAGES CXX)

set(CMAKE_BUILD_TYPE Release)
set(CMAKE_CXX_FLAGS_RELEASE "-DSPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_OFF -O3")

find_package(redGrapes REQUIRED CONFIG PATHS "${CMAKE_CURRENT_LIST_DIR}/../thirdparty/redGrapes/")
include_directories(SYSTEM ${redGrapes_INCLUDE_DIRS})

find_package(Threads REQUIRED)

add_executable(redgrapes src/redgrapes.cpp)
target_compile_features(redgrapes PUBLIC cxx_std_${redGrapes_CXX_STANDARD})
target_link_libraries(redgrapes PRIVATE redGrapes)
target_link_libraries(redgrapes PRIVATE Threads::Threads)
target_compile_options(redgrapes PRIVATE "-DNDEBUG")

add_executable(superglue src/superglue.cpp)
target_include_directories(superglue PRIVATE "${CMAKE_CURRENT_LIST_DIR}/../thirdparty/superglue/include")
target_include_directories(superglue PRIVATE "${CMAKE_CURRENT_LIST_DIR}/../thirdparty/.install/include")
target_link_directories(superglue PRIVATE "${CMAKE_CURRENT_LIST_DIR}/../thirdparty/.install/lib")
target_link_libraries(superglue hwloc pthread)

add_executable(quark src/quark.cpp)
target_include_directories(quark PRIVATE "${CMAKE_CURRENT_LIST_DIR}/../thirdparty/.install/include")
target_link_libraries(quark "${CMAKE_CURRENT_LIST_DIR}/../thirdparty/.install/lib/libquark.a")
target_link_directories(quark PRIVATE "${CMAKE_CURRENT_LIST_DIR}/../thirdparty/.install/lib")

target_link_libraries(quark hwloc pthread)

