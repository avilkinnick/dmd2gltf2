cmake_minimum_required(VERSION 3.10)

project(dmd2gltf LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(dependencies/fastgltf)

add_executable(dmd2gltf src/main.cpp)

target_link_libraries(dmd2gltf PRIVATE fastgltf::fastgltf)
