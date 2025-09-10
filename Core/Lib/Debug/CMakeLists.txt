cmake_minimum_required(VERSION 3.22)

add_library(debug_options STATIC
    src/swo_backend.cpp
    src/swo_terminal.cpp
)

target_include_directories(debug_options 
    PUBLIC 
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
    )

target_link_libraries(debug_options PUBLIC 
    )