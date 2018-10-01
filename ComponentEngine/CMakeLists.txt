cmake_minimum_required(VERSION 2.6)

set(project_name ComponentEngine)
project(${project_name})

set(HAVE_LIBC TRUE)

set(common_headers
    include/ComponentEngine/Engine.hpp
    include/ComponentEngine/DefaultMeshVertex.hpp
    include/ComponentEngine/Components/Camera.hpp
    include/ComponentEngine/Components/Transformation.hpp
)

set(common_source
    src/Engine.cpp
    src/Components/Transformation.cpp
)

include_directories(include)

set(source
    ${common_source}
)

set(headers
    ${common_headers}
)

include_directories(../../Renderer/renderer/include)
include_directories(../../EnteeZ/EnteeZ/include)

add_library(${project_name} STATIC ${source} ${headers})

target_link_libraries(${project_name} ../../../Renderer/build/renderer/Debug/Renderer)
target_link_libraries(${project_name} ../../../EnteeZ/build/EnteeZ/Debug/EnteeZ)


find_package(Vulkan)

if(Vulkan_FOUND)
	target_include_directories(${project_name} PRIVATE Vulkan::Vulkan)
	target_link_libraries(${project_name} Vulkan::Vulkan)
endif()