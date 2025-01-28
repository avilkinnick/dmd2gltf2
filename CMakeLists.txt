cmake_minimum_required(VERSION 3.10)

project(dmd2gltf LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(DMD_MODELS_DIR ${PROJECT_SOURCE_DIR}/models/dmd)
set(GLTF_MODELS_DIR ${PROJECT_SOURCE_DIR}/models/gltf)
set(DMD_ROUTES_DIR ${PROJECT_SOURCE_DIR}/routes/dmd)
set(GLTF_ROUTES_DIR ${PROJECT_SOURCE_DIR}/routes/gltf)

configure_file(cmake_defines.h.in ${PROJECT_SOURCE_DIR}/include/cmake_defines.h)

file(GLOB DMD2GLTF_SOURCES include/*.h src/*.cpp)

add_executable(dmd2gltf ${DMD2GLTF_SOURCES})

target_include_directories(dmd2gltf PRIVATE include)
