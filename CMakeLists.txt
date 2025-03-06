cmake_minimum_required(VERSION 3.10)

project(dmd2gltf LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(dmd2gltf
    ${PROJECT_SOURCE_DIR}/include/Application.h
    ${PROJECT_SOURCE_DIR}/include/Geometry.h
    ${PROJECT_SOURCE_DIR}/include/ModelConverter.h
    ${PROJECT_SOURCE_DIR}/include/RouteConverter.h
    ${PROJECT_SOURCE_DIR}/src/Application.cpp
    ${PROJECT_SOURCE_DIR}/src/main.cpp
    ${PROJECT_SOURCE_DIR}/src/ModelConverter.cpp
    ${PROJECT_SOURCE_DIR}/src/RouteConverter.cpp
)

target_include_directories(dmd2gltf PRIVATE
    ${PROJECT_SOURCE_DIR}/include
)
