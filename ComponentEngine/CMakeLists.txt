cmake_minimum_required(VERSION 2.6)

set(project_name ComponentEngine)
project(${project_name})

set(common_headers
    include/ComponentEngine/Engine.hpp
)

set(common_source
    src/Engine.cpp
)

include_directories(include)

set(source
    ${common_source}
)

set(headers
    ${common_headers}
)

add_library(${project_name} STATIC ${source} ${headers})