
cmake_minimum_required(VERSION 3.10.0)

project(LatencyBenchmark LANGUAGES CXX)

set(CMAKE_BUILD_TYPE Release)
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DSPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_OFF")

find_package(redGrapes REQUIRED)
#find_package(hwloc REQUIRED)
find_package(LAPACK REQUIRED)

add_executable(redgrapes redgrapes.cpp)
target_link_libraries(redgrapes redGrapes ${LAPACK_LIBRARIES})
target_compile_options(redgrapes PRIVATE ${LAPACK_LINKER_FLAGS})

add_executable(superglue superglue.cpp)
target_include_directories(superglue PRIVATE "${CMAKE_CURRENT_LIST_DIR}/../thirdparty/superglue/include")
target_link_libraries(superglue pthread ${LAPACK_LIBRARIES})
target_compile_options(superglue PRIVATE ${LAPACK_LINKER_FLAGS})

add_executable(quark quark.cpp)
target_include_directories(quark PRIVATE "${CMAKE_CURRENT_LIST_DIR}/../thirdparty/quark-0.9.0/install/include")
target_link_libraries(quark "${CMAKE_CURRENT_LIST_DIR}/../thirdparty/quark-0.9.0/install/lib/libquark.a")

target_link_directories(quark PRIVATE "${CMAKE_CURRENT_LIST_DIR}/../thirdparty/hwloc-2.7.1/install/lib/")
target_link_libraries(quark pthread ${LAPACK_LIBRARIES})
target_compile_options(quark PRIVATE ${LAPACK_LINKER_FLAGS})

