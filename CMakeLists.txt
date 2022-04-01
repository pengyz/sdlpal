# 目前仅支持unix最小构建，需要继续根据Makefile翻译可选项
cmake_minimum_required(VERSION 3.12)

set(CMAKE_CXX_STANDARD 11)
project(sdlpal)

add_subdirectory(${CMAKE_SOURCE_DIR}/3rd/SDL)

set(GTEST_DIR ${CMAKE_SOURCE_DIR}/3rd/googletest/googletest)
set(MIDI_CPPFILES ${CMAKE_SOURCE_DIR}/unix/native_midi.cpp)

file(GLOB ADPLUG_FILES CONFIGURE_DEPENDS "adplug/*.c" "adplug/*.cpp" "adplug/*.h")

add_library(adplug STATIC 
    ${ADPLUG_FILES}
)

target_include_directories(adplug PUBLIC
    ${CMAKE_SOURCE_DIR}/
    ${CMAKE_SOURCE_DIR}/adplug/
    ${CMAKE_SOURCE_DIR}/unix/
)

target_link_libraries(adplug
    SDL2-static
    SDL2main
)

file(GLOB OGGVORBIS_FILES CONFIGURE_DEPENDS "liboggvorbis/src/*.c" "liboggvorbis/src/*.cpp" "liboggvorbis/src/*.h")
add_library(oggvorbis STATIC
    ${OGGVORBIS_FILES}
)
target_include_directories(oggvorbis PUBLIC
    ${CMAKE_SOURCE_DIR}/liboggvorbis/include/ 
    ${CMAKE_SOURCE_DIR}/liboggvorbis/src/
)

file(GLOB OPUSFILE_FILES CONFIGURE_DEPENDS
    "libopusfile/src/*.c"
    "libopusfile/celt/*.c"
    "libopusfile/silk/*.c"
    "libopusfile/silk/float/*.c"

    "libopusfile/src/*.cpp"
    "libopusfile/celt/*.cpp"
    "libopusfile/silk/*.cpp"
    "libopusfile/silk/float/*.cpp"

    "libopusfile/src/*.h"
    "libopusfile/celt/*.h"
    "libopusfile/silk/*.h"
    "libopusfile/silk/float/*.h"
)

add_library(opusfile STATIC
    ${OPUSFILE_FILES}
)

target_include_directories(opusfile PUBLIC
    ${CMAKE_SOURCE_DIR}/libopusfile/include/
    ${CMAKE_SOURCE_DIR}/libopusfile/src/
    ${CMAKE_SOURCE_DIR}/libopusfile/celt/
    ${CMAKE_SOURCE_DIR}/libopusfile/silk/
    ${CMAKE_SOURCE_DIR}/libopusfile/silk/float/
)

target_compile_definitions(opusfile PRIVATE
    HAVE_CONFIG_H
)

target_link_libraries(opusfile
    oggvorbis
)


file(GLOB MAD_FILES CONFIGURE_DEPENDS "libmad/*.c" "libmad/*.cpp" "libmad/*.h")
add_library(mad STATIC 
    ${MAD_FILES}
)

target_link_libraries(mad PUBLIC
    SDL2-static
    SDL2main
)

file(GLOB TIMIDITY_FILES CONFIGURE_DEPENDS "timidity/*.c" "timidity/*.cpp" "timidity/*.h")
add_library(timidity STATIC
    ${TIMIDITY_FILES}
)

target_include_directories(timidity PRIVATE
    ${CMAKE_SOURCE_DIR}/
    ${CMAKE_SOURCE_DIR}/unix/
)

target_link_libraries(timidity PUBLIC
    SDL2-static
    SDL2main
)

file(GLOB SOURCE_FILES CONFIGURE_DEPENDS "*.c" "*.cpp" "*.h")

add_executable(sdlpal
    ${SOURCE_FILES}
    ${MIDI_CPPFILES}
    "unix/unix.cpp"
)

target_include_directories(sdlpal PRIVATE
    ${CMAKE_SOURCE_DIR}/timidity/
)

target_compile_definitions(sdlpal PRIVATE
    _GNU_SOURCE
    PAL_HAS_PLATFORM_SPECIFIC_UTILS
    PAL_NO_LAUNCH_UI    # no fltk launch ui
)

target_link_libraries(sdlpal PRIVATE
    adplug
    opusfile
    mad
    timidity
    m
    stdc++
    GL
)