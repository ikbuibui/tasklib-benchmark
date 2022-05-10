
cmake_minimum_required(VERSION 3.10.0)

project(redGrapesExamples LANGUAGES CXX)

find_package(redGrapes REQUIRED)

set(CXXFLAGS -O3)


add_executable(redgrapes redgrapes.cpp)
include_directories(redgrapes PRIVATE ${redGrapes_INCLUDE_DIRS})
target_link_libraries(redgrapes redGrapes)


add_executable(superglue superglue.cpp)
target_include_directories(superglue PRIVATE "${CMAKE_CURRENT_LIST_DIR}/../thirdparty/superglue/include")
target_link_libraries(superglue pthread)

