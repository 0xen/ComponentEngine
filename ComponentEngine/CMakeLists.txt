cmake_minimum_required(VERSION 2.6)

set(project_name ComponentEngine)
project(${project_name})

set(HAVE_LIBC TRUE)

set(common_headers
    # External
    include/ComponentEngine/tiny_obj_loader.h
    include/ComponentEngine/obj_loader.h
    ../../lodepng/lodepng.h
    ../third_party/cimg/CImg.h


    include/ComponentEngine/Engine.hpp
    include/ComponentEngine/Common.hpp
    include/ComponentEngine/ThreadHandler.hpp
    include/ComponentEngine/ThreadManager.hpp
    include/ComponentEngine/DefaultMeshVertex.hpp
    include/ComponentEngine/UIManager.hpp
    include/ComponentEngine/Type.hpp
    include/ComponentEngine/TemplateType.hpp
    include/ComponentEngine/PhysicsWorld.hpp

    include/x264_capture.h

)

set(common_source
    # External
    ../../lodepng/lodepng.cpp
    
    src/obj_loader.cpp
    src/Engine.cpp
    src/Common.cpp
    src/ThreadHandler.cpp
    src/ThreadManager.cpp
    src/UIManager.cpp
    src/PhysicsWorld.cpp

    src/x264_capture.cpp
)


source_group("Source Files\\ComponentEngine" FILES ${common_source})
source_group("Header Files\\ComponentEngine" FILES ${common_headers})

set(component_headers
    include/ComponentEngine/Components/UI.hpp
    include/ComponentEngine/Components/IO.hpp
    include/ComponentEngine/Components/TransferBuffers.hpp
    include/ComponentEngine/Components/ComponentMessages.hpp
    include/ComponentEngine/Components/MsgRecive.hpp
    include/ComponentEngine/Components/MsgSend.hpp
    include/ComponentEngine/Components/Camera.hpp
    include/ComponentEngine/Components/Mesh.hpp
    include/ComponentEngine/Components/Transformation.hpp
    include/ComponentEngine/Components/Renderer.hpp
    include/ComponentEngine/Components/Logic.hpp
    include/ComponentEngine/Components/Rigidbody.hpp
    include/ComponentEngine/Components/ICollisionShape.hpp
    include/ComponentEngine/Components/BoxCollision.hpp
    include/ComponentEngine/Components/SphereCollision.hpp
    include/ComponentEngine/Components/Light.hpp
)

set(component_source
    src/Components/UI.cpp
    src/Components/MsgRecive.cpp
    src/Components/MsgSend.cpp
    src/Components/Mesh.cpp
    src/Components/Camera.cpp
    src/Components/Transformation.cpp
    src/Components/Renderer.cpp
    src/Components/Rigidbody.cpp
    src/Components/ICollisionShape.cpp
    src/Components/BoxCollision.cpp
    src/Components/SphereCollision.cpp
    src/Components/Light.cpp
)


list(APPEND common_source ${component_source})
list(APPEND common_headers ${component_headers})


source_group("Source Files\\ComponentEngine\\Components" FILES ${component_source})
source_group("Header Files\\ComponentEngine\\Components" FILES ${component_headers})

set(ui_headers
    include/ComponentEngine/UI/MenuElement.hpp

    include/ComponentEngine/UI/UIBase.hpp
    include/ComponentEngine/UI/UIManager.hpp
    include/ComponentEngine/UI/Threading.hpp
    include/ComponentEngine/UI/Console.hpp
    include/ComponentEngine/UI/SceneHierarchy.hpp
    include/ComponentEngine/UI/ComponentHierarchy.hpp
    include/ComponentEngine/UI/Explorer.hpp
    include/ComponentEngine/UI/EditorState.hpp
    include/ComponentEngine/UI/SceneWindow.hpp
    include/ComponentEngine/UI/PlayWindow.hpp
    include/ComponentEngine/UI/UITemplate.hpp
)

set(ui_source
    src/UI/MenuElement.cpp

    src/UI/UIBase.cpp
    src/UI/UIManager.cpp
    src/UI/Threading.cpp
    src/UI/Console.cpp
    src/UI/SceneHierarchy.cpp
    src/UI/ComponentHierarchy.cpp
    src/UI/Explorer.cpp
    src/UI/EditorState.cpp
    src/UI/SceneWindow.cpp
    src/UI/PlayWindow.cpp
    src/UI/UITemplate.cpp
)


list(APPEND common_source ${ui_source})
list(APPEND common_headers ${ui_headers})


source_group("Source Files\\ComponentEngine\\UI" FILES ${ui_source})
source_group("Header Files\\ComponentEngine\\UI" FILES ${ui_headers})








file(GLOB x264lib_h
    ../third_party/x264/*.h
)

list(APPEND common_headers ${x264lib_h})


source_group("Header Files\\x265" FILES ${x264lib_h})





#file(GLOB FFMPEG_CODEC_H
#    ../third_party/ffmpeg-4.2.2/libavcodec/*.h
#)
#file(GLOB FFMPEG_CODEC_C
#    ../third_party/ffmpeg-4.2.2/libavcodec/*.c
#)
#
#file(GLOB FFMPEG_FORMAT_H
#    ../third_party/ffmpeg-4.2.2/libavformat/*.h
#)
#file(GLOB FFMPEG_FORMAT_C
#    ../third_party/ffmpeg-4.2.2/libavformat/*.c
#)
#
#file(GLOB FFMPEG_UTIL_H
#    ../third_party/ffmpeg-4.2.2/libavutil/*.h
#)
#file(GLOB FFMPEG_UTIL_C
#    ../third_party/ffmpeg-4.2.2/libavutil/*.c
#)
#
#file(GLOB FFMPEG_CODECDEVICE_H
#    ../third_party/ffmpeg-4.2.2/libavdevice/*.h
#)
#file(GLOB FFMPEG_CODECDEVICE_C
#    ../third_party/ffmpeg-4.2.2/libavdevice/*.c
#)
#
#
#
#
#list(APPEND common_source ${FFMPEG_CODEC_C})
#list(APPEND common_headers ${FFMPEG_CODEC_H})
#
#list(APPEND common_source ${FFMPEG_FORMAT_C})
#list(APPEND common_headers ${FFMPEG_FORMAT_H})
#
#list(APPEND common_source ${FFMPEG_UTIL_C})
#list(APPEND common_headers ${FFMPEG_UTIL_H})
#
#list(APPEND common_source ${FFMPEG_CODECDEVICE_C})
#list(APPEND common_headers ${FFMPEG_CODECDEVICE_H})


#source_group("Source Files\\FFMPEG\\libavcodec" FILES ${FFMPEG_CODEC_C})
#source_group("Header Files\\FFMPEG\\libavcodec" FILES ${FFMPEG_CODEC_H})
#
#source_group("Source Files\\FFMPEG\\libavformat" FILES ${FFMPEG_FORMAT_C})
#source_group("Header Files\\FFMPEG\\libavformat" FILES ${FFMPEG_FORMAT_H})
#
#source_group("Source Files\\FFMPEG\\libavutil" FILES ${FFMPEG_UTIL_C})
#source_group("Header Files\\FFMPEG\\libavutil" FILES ${FFMPEG_UTIL_H})
#
#source_group("Source Files\\FFMPEG\\libavdevice" FILES ${FFMPEG_CODECDEVICE_C})
#source_group("Header Files\\FFMPEG\\libavdevice" FILES ${FFMPEG_CODECDEVICE_H})







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
)
list(APPEND source ${imgui_src})
list(APPEND headers ${imgui_headers})

source_group("Source Files\\External\\imgui" FILES ${imgui_src})
source_group("Header Files\\External\\imgui" FILES ${imgui_headers})


include_directories(../../VK-EZ/renderer/include)
include_directories(../../EnteeZ/EnteeZ/include)

add_library(${project_name} STATIC ${source} ${headers})

target_link_libraries( ${project_name} debug ../../../VK-EZ/build/renderer/Debug/Renderer)
target_link_libraries( ${project_name} optimized ../../../VK-EZ/build/renderer/Release/Renderer)


target_link_libraries( ${project_name} debug ../../../EnteeZ/build/EnteeZ/Debug/EnteeZ)
target_link_libraries( ${project_name} optimized ../../../EnteeZ/build/EnteeZ/Release/EnteeZ)


target_link_libraries(${project_name} BulletDynamics)
target_link_libraries(${project_name} BulletCollision)
target_link_libraries(${project_name} LinearMath)
target_link_libraries(${project_name} jpeg)

#target_link_libraries(${project_name} glslang SPIRV)

find_package(Vulkan)

if(Vulkan_FOUND)
    target_include_directories(${project_name} PRIVATE Vulkan::Vulkan)
    target_link_libraries(${project_name} Vulkan::Vulkan)
endif()


target_include_directories(${project_name} PRIVATE ../third_party/x264/)
target_link_libraries(${project_name} ../../third_party/x264/libx264)

#target_include_directories(${project_name} PRIVATE ../third_party/ffmpeg-4.2.2/)