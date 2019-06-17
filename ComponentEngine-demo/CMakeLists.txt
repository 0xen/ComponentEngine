cmake_minimum_required(VERSION 2.6)

set(project_name ComponentEngine-demo)
project(${project_name})

set(
    CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG
    ${CMAKE_HOME_DIRECTORY}/ComponentEngine-demo/debug-bin
    )
set(
    CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE
    ${CMAKE_HOME_DIRECTORY}/ComponentEngine-demo/bin
    )


set(HAVE_LIBC TRUE)
set(src
    src/main.cpp
    src/KeyboardMovment.cpp
)
 
set(headers
    include/KeyboardMovment.hpp
)

include_directories(include)
include_directories(../ComponentEngine/include)
include_directories(../../VK-EZ/renderer/include)
include_directories(../../EnteeZ/EnteeZ/include)


add_executable(${project_name} ${src} ${headers})


set_property(TARGET ${project_name} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/ComponentEngine-demo/debug-bin")

target_link_libraries(${project_name} ComponentEngine)

target_link_libraries( ${project_name} debug ../../../VK-EZ/build/renderer/Debug/Renderer)
target_link_libraries( ${project_name} optimized ../../../VK-EZ/build/renderer/Release/Renderer)

target_link_libraries( ${project_name} debug ../../../EnteeZ/build/EnteeZ/Debug/EnteeZ)
target_link_libraries( ${project_name} optimized ../../../EnteeZ/build/EnteeZ/Release/EnteeZ)

target_link_libraries(${project_name} SDL2main)
target_link_libraries(${project_name} SDL2-static)

target_link_libraries(${project_name} BulletDynamics)
target_link_libraries(${project_name} BulletCollision)
target_link_libraries(${project_name} LinearMath)

#target_link_libraries(${project_name} glslang SPIRV)



find_package(Vulkan)


if(Vulkan_FOUND)
	target_include_directories(${project_name} PRIVATE Vulkan::Vulkan)
	target_link_libraries(${project_name} Vulkan::Vulkan)
endif()

