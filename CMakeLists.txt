cmake_minimum_required(VERSION 4.2.1)
project(vulkan)

set(CMAKE_CXX_STANDERD 20)
set(CMAKE_CXX_STANDERD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(${PROJECT_NAME})
target_sources(${PROJECT_NAME} PRIVATE src/main.cpp)







