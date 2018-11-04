cmake_minimum_required(VERSION 2.6)

set(project_name ComponentEngine-demo)
project(${project_name})

set(HAVE_LIBC TRUE)

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

target_link_libraries( ${project_name} debug ../../../Renderer/build/renderer/Debug/Renderer)
target_link_libraries( ${project_name} optimized ../../../Renderer/build/renderer/Release/Renderer)

target_link_libraries( ${project_name} debug ../../../EnteeZ/build/EnteeZ/Debug/EnteeZ)
target_link_libraries( ${project_name} optimized ../../../EnteeZ/build/EnteeZ/Release/EnteeZ)

target_link_libraries(${project_name} SDL2main)
target_link_libraries(${project_name} SDL2-static)


find_package(Vulkan)

if(Vulkan_FOUND)
	target_include_directories(${project_name} PRIVATE Vulkan::Vulkan)
	target_link_libraries(${project_name} Vulkan::Vulkan)
endif()

