cmake_minimum_required(VERSION 2.6)

set(project_name demo)
project(${project_name})

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

add_subdirectory(third_party/SDL2)

set_target_properties (SDL2 PROPERTIES FOLDER "Third Party/SDL2")
set_target_properties (SDL2main PROPERTIES FOLDER "Third Party/SDL2")
set_target_properties (SDL2-static PROPERTIES FOLDER "Third Party/SDL2")
set_target_properties (uninstall PROPERTIES FOLDER "Third Party/SDL2")

include_directories(third_party/SDL2/include)
include_directories(../lodepng)
include_directories(third_party/imgui)
include_directories(../Renderer/renderer)
include_directories(../EnteeZ/EnteeZ)

add_subdirectory(third_party/glm)
include_directories(third_party/glm)

add_subdirectory(ComponentEngine)
add_subdirectory(ComponentEngine-demo)
