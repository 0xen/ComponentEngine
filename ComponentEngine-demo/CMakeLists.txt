cmake_minimum_required(VERSION 2.6)

set(project_name ComponentEngine-demo)
project(${project_name})

set(src
    src/main.cpp
)

set(headers
)

include_directories(include)
include_directories(../ComponentEngine/include)
include_directories(../../Renderer/renderer/include)
include_directories(../../EnteeZ/EnteeZ/include)


add_executable(${project_name} ${src} ${headers})

target_link_libraries(${project_name} ComponentEngine)
target_link_libraries(${project_name} ../../../Renderer/build/renderer/Debug/Renderer)
target_link_libraries(${project_name} ../../../EnteeZ/build/EnteeZ/Debug/EnteeZ)