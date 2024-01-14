cmake_minimum_required(VERSION 3.12)

project(script_viewer)

file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.h
)

add_executable(${PROJECT_NAME}
    ${SRC_FILES}
    ${CMAKE_SOURCE_DIR}/src/engine/pal_script.cpp
    ${CMAKE_SOURCE_DIR}/src/engine/pal_script.h
    ${CMAKE_SOURCE_DIR}/src/engine/script_opcode.h
)

target_include_directories(${PROJECT_NAME} PUBLIC
    ${CMAKE_SOURCE_DIR}/src/
)

target_link_libraries(${PROJECT_NAME} PUBLIC
    game
    GL
)