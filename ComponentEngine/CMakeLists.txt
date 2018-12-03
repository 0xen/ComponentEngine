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
)

set(common_source
    # External
    src/pugixml.cpp
    ../../lodepng/lodepng.cpp

    src/Engine.cpp
    src/Common.cpp
    src/ThreadHandler.cpp
    src/UIManager.cpp
)


source_group("Source Files\\ComponentEngine" FILES ${common_source})
source_group("Header Files\\ComponentEngine" FILES ${common_headers})

set(component_headers
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

set(component_source
    src/Components/UI.cpp
    src/Components/MsgRecive.cpp
    src/Components/MsgSend.cpp
    src/Components/Mesh.cpp
    src/Components/Transformation.cpp
    src/Components/ParticalSystem.cpp
    src/Components/Renderer.cpp
    src/Components/Indestructable.cpp
)


list(APPEND common_source ${component_source})
list(APPEND common_headers ${component_headers})


source_group("Source Files\\ComponentEngine\\Components" FILES ${component_source})
source_group("Header Files\\ComponentEngine\\Components" FILES ${component_headers})



set(event_headers
    include/ComponentEngine/Events/KeyboardEvent.hpp
    include/ComponentEngine/Events/MouseEvent.hpp
)

set(event_source

)


list(APPEND common_source ${event_source})
list(APPEND common_headers ${event_headers})


source_group("Source Files\\ComponentEngine\\Events" FILES ${event_source})
source_group("Header Files\\ComponentEngine\\Events" FILES ${event_headers})


set(datastructure_headers
    include/ComponentEngine/DataStructure/LinkedList.hpp
)

set(datastructure_source

)


list(APPEND common_source ${datastructure_source})
list(APPEND common_headers ${datastructure_headers})


source_group("Source Files\\ComponentEngine\\DataStructure" FILES ${datastructure_source})
source_group("Header Files\\ComponentEngine\\DataStructure" FILES ${datastructure_headers})



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