cmake_minimum_required(VERSION 2.6)

set(project_name ComponentEngine)
project(${project_name})

set(HAVE_LIBC TRUE)

set(common_headers
    # External
    include/ComponentEngine/pugixml.hpp
    include/ComponentEngine/pugiconfig.hpp
    include/ComponentEngine/tiny_obj_loader.h
    ../../lodepng/lodepng.h


    include/ComponentEngine/Engine.hpp
    include/ComponentEngine/Common.hpp
    include/ComponentEngine/ThreadHandler.hpp
    include/ComponentEngine/DefaultMeshVertex.hpp
    include/ComponentEngine/UIManager.hpp
    include/ComponentEngine/Components/UI.hpp
    include/ComponentEngine/Components/ComponentMessages.hpp
    include/ComponentEngine/Components/MsgRecive.hpp
    include/ComponentEngine/Components/MsgSend.hpp
    include/ComponentEngine/Components/Camera.hpp
    include/ComponentEngine/Components/Mesh.hpp
    include/ComponentEngine/Components/Transformation.hpp
    include/ComponentEngine/Components/ParticalSystem.hpp
    include/ComponentEngine/Components/Renderer.hpp
    include/ComponentEngine/Components/Logic.hpp
    include/ComponentEngine/Components/Indestructable.hpp
)

set(common_source
    # External
    src/pugixml.cpp
    ../../lodepng/lodepng.cpp

    src/Engine.cpp
    src/Common.cpp
    src/ThreadHandler.cpp
    src/UIManager.cpp
    src/Components/UI.cpp
    src/Components/MsgRecive.cpp
    src/Components/MsgSend.cpp
    src/Components/Mesh.cpp
    src/Components/Transformation.cpp
    src/Components/ParticalSystem.cpp
    src/Components/Renderer.cpp
    src/Components/Indestructable.cpp
)

include_directories(include)

set(source
    ${common_source}
)

set(headers
    ${common_headers}
)


# Load imgui
set(imgui_src
    ../third_party/imgui/imgui.cpp
    ../third_party/imgui/imgui_demo.cpp
    ../third_party/imgui/imgui_draw.cpp
    ../third_party/imgui/imgui_widgets.cpp
    ../third_party/imgui/examples/imgui_impl_sdl.cpp
)
set(imgui_headers
    ../third_party/imgui/imconfig.h
    ../third_party/imgui/imgui.h
    ../third_party/imgui/imgui_internal.h
    ../third_party/imgui/stb_rect_pack.h
    ../third_party/imgui/stb_textedit.h
    ../third_party/imgui/stb_truetype.h
    ../third_party/imgui/examples/imgui_impl_sdl.h
)
list(APPEND source ${imgui_src})
list(APPEND headers ${imgui_headers})

source_group("Source Files\\External\\imgui" FILES ${imgui_src})
source_group("Header Files\\External\\imgui" FILES ${imgui_headers})


include_directories(../../Renderer/renderer/include)
include_directories(../../EnteeZ/EnteeZ/include)

add_library(${project_name} STATIC ${source} ${headers})

target_link_libraries( ${project_name} debug ../../../Renderer/build/renderer/Debug/Renderer)
target_link_libraries( ${project_name} optimized ../../../Renderer/build/renderer/Release/Renderer)

target_link_libraries(${project_name} ../../../Renderer/build/renderer/Release/Renderer)
target_link_libraries(${project_name} ../../../EnteeZ/build/EnteeZ/Release/EnteeZ)


find_package(Vulkan)

if(Vulkan_FOUND)
	target_include_directories(${project_name} PRIVATE Vulkan::Vulkan)
	target_link_libraries(${project_name} Vulkan::Vulkan)
endif()