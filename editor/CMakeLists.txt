cmake_minimum_required(VERSION 3.12)

project(sdlpal_editor)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_C_STANDARD 99)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fno-strict-aliasing -D_GNU_SOURCE -g -Wall")


add_subdirectory(deps/SDL EXCLUDE_FROM_ALL)
add_subdirectory(deps/stb_image EXCLUDE_FROM_ALL)
add_subdirectory(tools/script_viewer)


# oggvorbis
file(GLOB_RECURSE  oggvorbis_FILES
    ${CMAKE_SOURCE_DIR}/../liboggvorbis/*.c
)

add_library(oggvorbis STATIC
    ${oggvorbis_FILES}
)

target_include_directories(oggvorbis PUBLIC
    ${CMAKE_SOURCE_DIR}/../liboggvorbis/include/
    ${CMAKE_SOURCE_DIR}/../liboggvorbis/src/
)

file(GLOB opusfile_files
    ${CMAKE_SOURCE_DIR}/../libopusfile/src/*.c
    ${CMAKE_SOURCE_DIR}/../libopusfile/celt/*.c
    ${CMAKE_SOURCE_DIR}/../libopusfile/silk/*.c
    ${CMAKE_SOURCE_DIR}/../libopusfile/silk/float/*.c

    ${CMAKE_SOURCE_DIR}/../libopusfile/src/*.cpp
    ${CMAKE_SOURCE_DIR}/../libopusfile/celt/*.cpp
    ${CMAKE_SOURCE_DIR}/../libopusfile/silk/*.cpp
    ${CMAKE_SOURCE_DIR}/../libopusfile/silk/float/*.cpp
)

# libopusfile
add_library(opusfile STATIC
    ${opusfile_files}
)

target_compile_definitions(opusfile PRIVATE
    USE_ALLOCA
    CUSTOM_MODES
    OPUS_BUILD
)

target_include_directories(opusfile PUBLIC
    ${CMAKE_SOURCE_DIR}/../libopusfile/include/
    ${CMAKE_SOURCE_DIR}/../libopusfile/celt/
    ${CMAKE_SOURCE_DIR}/../libopusfile/src/
    ${CMAKE_SOURCE_DIR}/../libopusfile/silk/
    ${CMAKE_SOURCE_DIR}/../libopusfile/silk/float/
    ${CMAKE_SOURCE_DIR}/../liboggvorbis/include/
)

# timidity
file(GLOB_RECURSE timidity_FILES
    ${CMAKE_SOURCE_DIR}/../timidity/*.c
)
add_library(timidity STATIC
    ${timidity_FILES}
)

target_include_directories(timidity PUBLIC
    ${CMAKE_SOURCE_DIR}/../timidity/
    ${CMAKE_SOURCE_DIR}/../
    ${CMAKE_SOURCE_DIR}/../unix/
)

target_link_libraries(timidity PUBLIC
    SDL2-static
)

# native_midi
file(GLOB_RECURSE native_midi_FILES
    ${CMAKE_SOURCE_DIR}/../native_midi/*.c
)
add_library(native_midi STATIC
    ${native_midi_FILES}
)

target_link_libraries(native_midi PUBLIC
    SDL2-static
)

# libmad
file(GLOB_RECURSE mad_FILES
    ${CMAKE_SOURCE_DIR}/../libmad/*.c
)
add_library(mad STATIC
    ${mad_FILES}
)

target_link_libraries(mad PUBLIC
    SDL2-static
)

# adplug
file(GLOB_RECURSE adplug_FILES
    ${CMAKE_SOURCE_DIR}/../adplug/*.c
    ${CMAKE_SOURCE_DIR}/../adplug/*.cpp
)
add_library(adplug STATIC
    ${adplug_FILES}
)

target_link_libraries(adplug PUBLIC
    SDL2-static
)

target_include_directories(adplug PUBLIC
    ${CMAKE_SOURCE_DIR}/../unix/
    ${CMAKE_SOURCE_DIR}/../
)

# add game library
add_library(game STATIC
    ../game.c
    ../global.c
    ../util.c
    ../palcommon.c
    ../uigame.c
    ../res.c
    ../input.c
    ../font.c
    ../ui.c
    ../text.c
    ../script.c
    ../yj1.c
    ../palcfg.c
    ../video.c
    ../map.c
    ../audio.c
    ../video_glsl.c
    ../palette.c
    ../play.c
    ../battle.c
    ../scene.c
    ../fight.c
    ../rngplay.c
    ../ending.c
    ../overlay.c
    ../resampler.c
    ../midi.c
    ../sound.c
    ../mp3play.c
    ../oggplay.c
    ../opusplay.c
    ../rixplay.cpp
    ../midi_timidity.c
    ../midi_tsf.c
    ../aviplay.c
    ../glslp.c
    ../mini_glloader.c
    ../uibattle.c
    ../itemmenu.c
    ../unix/native_midi.cpp
    ../magicmenu.c
)

target_include_directories(game PUBLIC
    ${CMAKE_SOURCE_DIR}/../
    ${CMAKE_SOURCE_DIR}/deps/SDL/include/
    ${CMAKE_SOURCE_DIR}/../unix/
)

target_link_libraries(game PUBLIC
    SDL2-static
    SDL2main
    oggvorbis
    opusfile
    timidity
    native_midi
    mad
    adplug
)

set(IMGUI_PATH ${CMAKE_SOURCE_DIR}/deps/imgui)

# imgui_sdlrenderer
add_library(imgui_sdlrenderer STATIC
    ${IMGUI_PATH}/backends/imgui_impl_sdlrenderer2.cpp
    ${IMGUI_PATH}/backends/imgui_impl_sdlrenderer2.h
    ${IMGUI_PATH}/backends/imgui_impl_sdl2.cpp
    ${IMGUI_PATH}/backends/imgui_impl_sdl2.h
    
    ${IMGUI_PATH}/misc/cpp/imgui_stdlib.cpp
    ${IMGUI_PATH}/misc/cpp/imgui_stdlib.h
    ${IMGUI_PATH}/imgui.h
    ${IMGUI_PATH}/imgui.cpp
    ${IMGUI_PATH}/imgui_draw.cpp
    ${IMGUI_PATH}/imgui_tables.cpp
    ${IMGUI_PATH}/imgui_widgets.cpp
    ${IMGUI_PATH}/imgui_demo.cpp
)

target_include_directories(imgui_sdlrenderer PUBLIC
    ${IMGUI_PATH}/backends/
    ${IMGUI_PATH}/
    ${IMGUI_PATH}/misc/cpp/
)

target_link_libraries(imgui_sdlrenderer PUBLIC
    SDL2-static
    SDL2main
)

file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS 
    src/*.cpp
    src/*.c
    src/*.h
)


# editor executable
add_executable(${PROJECT_NAME}
    ${SRC_FILES}
)

target_include_directories(${PROJECT_NAME} PUBLIC
    ${CMAKE_SOURCE_DIR}/src/
)

target_link_libraries(${PROJECT_NAME} PUBLIC
    game
    GL
    imgui_sdlrenderer
    stb_image
)