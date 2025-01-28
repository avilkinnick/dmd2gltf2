cmake_minimum_required(VERSION 3.10)

project(dmd2gltf LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(dependencies/fastgltf)

file(GLOB DMD2GLTF_SOURCES include/*.h src/*.cpp)

add_executable(dmd2gltf ${DMD2GLTF_SOURCES})

target_link_libraries(dmd2gltf PRIVATE fastgltf::fastgltf)

target_include_directories(dmd2gltf PRIVATE include)
