
cmake_minimum_required(VERSION 3.10.0)

project(LatencyBenchmark LANGUAGES CXX)

set(CMAKE_BUILD_TYPE Release)
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

find_package(redGrapes REQUIRED)
find_package(LAPACK REQUIRED)

## SuperGlue doesn't like -DNDEBUG.
## Using -DNDEBUG with SuperGlue will result in freezes when using more than one worker-thread.

set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DSPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_OFF")


add_executable(superglue src/superglue.cpp)
target_include_directories(superglue PRIVATE "${CMAKE_CURRENT_LIST_DIR}/../thirdparty/superglue/include")
target_include_directories(superglue PRIVATE "${CMAKE_CURRENT_LIST_DIR}/../thirdparty/.install/include")
target_link_directories(superglue PRIVATE "${CMAKE_CURRENT_LIST_DIR}/../thirdparty/.install/lib")
target_link_libraries(superglue hwloc pthread ${LAPACK_LIBRARIES})
target_compile_options(superglue PRIVATE ${LAPACK_LINKER_FLAGS})

add_executable(redgrapes src/redgrapes.cpp)
target_include_directories(redGrapes PRIVATE "${CMAKE_CURRENT_LIST_DIR}/../thirdparty/.install/include")
target_link_directories(redgrapes PRIVATE "${CMAKE_CURRENT_LIST_DIR}/../thirdparty/.install/lib")
target_link_libraries(redgrapes redGrapes ${LAPACK_LIBRARIES})
target_compile_options(redgrapes PRIVATE ${LAPACK_LINKER_FLAGS})
target_compile_options(redgrapes PRIVATE "-DNDEBUG")

add_executable(quark src/quark.cpp)
target_include_directories(quark PRIVATE "${CMAKE_CURRENT_LIST_DIR}/../thirdparty/.install/include")
target_link_libraries(quark "${CMAKE_CURRENT_LIST_DIR}/../thirdparty/.install/lib/libquark.a")
target_link_directories(quark PRIVATE "${CMAKE_CURRENT_LIST_DIR}/../thirdparty/.install/lib")
target_link_libraries(quark hwloc pthread ${LAPACK_LIBRARIES})
target_compile_options(quark PRIVATE ${LAPACK_LINKER_FLAGS})

