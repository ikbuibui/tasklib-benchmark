
cmake_minimum_required(VERSION 3.10.0)

project(TasklibBenchmark LANGUAGES CXX)

set(CMAKE_BUILD_TYPE Release)
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} -DSPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_TRACE -O3")

find_package(redGrapes REQUIRED)

add_executable(redgrapes src/redgrapes.cpp)
target_include_directories(redGrapes PRIVATE "${CMAKE_CURRENT_LIST_DIR}/../thirdparty/.install/include")
target_link_directories(redgrapes PRIVATE "${CMAKE_CURRENT_LIST_DIR}/../thirdparty/.install/lib")
target_link_libraries(redgrapes redGrapes)
target_compile_options(redgrapes PRIVATE "-DNDEBUG")

add_executable(superglue src/superglue.cpp)
target_include_directories(superglue PRIVATE "${CMAKE_CURRENT_LIST_DIR}/../thirdparty/superglue/include")
target_include_directories(superglue PRIVATE "${CMAKE_CURRENT_LIST_DIR}/../thirdparty/.install/include")
target_link_directories(superglue PRIVATE "${CMAKE_CURRENT_LIST_DIR}/../thirdparty/.install/lib")
target_link_libraries(superglue hwloc pthread)

# add_executable(quark src/quark.cpp)
# target_include_directories(quark PRIVATE "${CMAKE_CURRENT_LIST_DIR}/../thirdparty/.install/include")
# target_link_libraries(quark "${CMAKE_CURRENT_LIST_DIR}/../thirdparty/.install/lib/libquark.a")
# target_link_directories(quark PRIVATE "${CMAKE_CURRENT_LIST_DIR}/../thirdparty/.install/lib")

# target_link_libraries(quark hwloc pthread)

