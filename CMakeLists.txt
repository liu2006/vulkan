cmake_minimum_required(VERSION 4.2.3)
project(vulkan)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(${PROJECT_NAME})
target_sources(${PROJECT_NAME} PRIVATE src/main.cpp)

add_library(lib SHARED)
target_sources(lib
    PRIVATE
    src/application.cpp
    src/initVulkan.cpp

    PUBLIC
    FILE_SET CXX_MODULES
    
    FILES
    modules/application.cpp
    modules/internal.cpp
    modules/validationLayers.cpp
)
list(APPEND CMAKE_PREFIX_PATH "$ENV{HOME}/vendored/sdl3") 
list(APPEND CMAKE_PREFIX_PATH "$ENV{HOME}/vendored/1.4.341.0/x86_64") 
find_package(SDL3 CONFIG REQUIRED)
find_package(Vulkan REQUIRED)

target_include_directories(${PROJECT_NAME} PRIVATE ${SDL3_INCLUDE_DIRS} ${Vulkan_INCLUDE_DIRS})
target_link_libraries(lib PUBLIC SDL3::SDL3 Vulkan::Vulkan)
target_link_libraries(${PROJECT_NAME} PRIVATE lib)

install(TARGETS ${PROJECT_NAME} lib
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)


