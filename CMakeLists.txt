cmake_minimum_required(VERSION 2.6)

set(project_name demo)
project(${project_name})

set_property(GLOBAL PROPERTY USE_FOLDERS ON)


include_directories(../lodepng)
include_directories(third_party/imgui)
include_directories(../Renderer/renderer)
include_directories(../EnteeZ/EnteeZ)

add_subdirectory(third_party/glm)
include_directories(third_party/glm)

add_subdirectory(ComponentEngine)
add_subdirectory(ComponentEngine-demo)
