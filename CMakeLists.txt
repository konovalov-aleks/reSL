cmake_minimum_required(VERSION 3.13)

project(reSL)

file(GLOB_RECURSE HEADERS "*.h")

set(SOURCES
    src/game/dialog.cpp
    src/game/draw_header.cpp
    src/game/drawing.cpp
    src/game/entrance.cpp
    src/game/game_data.cpp
    src/game/game_over.cpp
    src/game/header.cpp
    src/game/impasse.cpp
    src/game/init.cpp
    src/game/io_status.cpp
    src/game/keyboard.cpp
    src/game/load_game.cpp
    src/game/main_loop.cpp
    src/game/main_menu.cpp
    src/game/melody.cpp
    src/game/mouse/construction_mode.cpp
    src/game/mouse/management_mode.cpp
    src/game/mouse/mouse.cpp
    src/game/mouse/mouse_state.cpp
    src/game/move_trains.cpp
    src/game/rail.cpp
    src/game/records.cpp
    src/game/resources/allowed_cursor_rail_types.cpp
    src/game/resources/carriage_bias.cpp
    src/game/resources/chunk_bounding_boxes.cpp
    src/game/resources/dispatcher_glyph.cpp
    src/game/resources/entrance.cpp
    src/game/resources/entrance_rails.cpp
    src/game/resources/glyph_empty_background.cpp
    src/game/resources/impasse_glyph.cpp
    src/game/resources/movement_paths.cpp
    src/game/resources/rail_connection_bias.cpp
    src/game/resources/rail_glyph.cpp
    src/game/resources/rail_info.cpp
    src/game/resources/rail_type_meta.cpp
    src/game/resources/semaphore_glyph.cpp
    src/game/resources/semaphore_glyph_bias.cpp
    src/game/resources/static_object_glyph.cpp
    src/game/resources/text_glyphs.cpp
    src/game/resources/train_finished_exclamation_glyph.cpp
    src/game/resources/train_glyph.cpp
    src/game/resources/train_specification.cpp
    src/game/road_construction.cpp
    src/game/semaphore.cpp
    src/game/static_object.cpp
    src/game/status_bar.cpp
    src/game/switch.cpp
    src/game/train.cpp
    src/graphics/animation.cpp
    src/graphics/drawing.cpp
    src/graphics/glyph.cpp
    src/graphics/text.cpp
    src/graphics/vga.cpp
    src/main.cpp
    src/system/active_sleep.cpp
    src/system/buffer.cpp
    src/system/driver/sdl/audio.cpp
    src/system/driver/sdl/driver.cpp
    src/system/driver/sdl/video.cpp
    src/system/exit.cpp
    src/system/random.cpp
    src/system/read_file.cpp
    src/system/sound.cpp
    src/system/time.cpp
    src/tasks/scheduler.cpp
    src/tasks/task.cpp
)

macro(GroupSources curdir)
   file(GLOB children RELATIVE ${PROJECT_SOURCE_DIR}/${curdir} ${PROJECT_SOURCE_DIR}/${curdir}/*)
   foreach(child ${children})
          if(IS_DIRECTORY ${PROJECT_SOURCE_DIR}/${curdir}/${child})
            GroupSources(${curdir}/${child})
          else()
            string(REPLACE "/" "\\" groupname ${curdir})
            source_group(${groupname} FILES ${PROJECT_SOURCE_DIR}/${curdir}/${child})
          endif()
   endforeach()
endmacro()

GroupSources(src)

add_executable(resl ${SOURCES} ${HEADERS})

set_property(TARGET resl PROPERTY CXX_STANDARD 20)

if (EMSCRIPTEN)
    target_compile_options(resl PRIVATE "-sUSE_SDL=2")
    target_link_options(resl PRIVATE
        "-sASYNCIFY"
        "-sUSE_SDL=2"
        "--preload-file=${CMAKE_CURRENT_SOURCE_DIR}/resources/PLAY.7@PLAY.7"
        "--preload-file=${CMAKE_CURRENT_SOURCE_DIR}/resources/GAMEOVER.7@GAMEOVER.7"
        "--preload-file=${CMAKE_CURRENT_SOURCE_DIR}/resources/DEMO_A@DEMO_A")
else()
    find_package(SDL2 REQUIRED CONFIG REQUIRED COMPONENTS SDL2)
    find_package(SDL2 REQUIRED CONFIG COMPONENTS SDL2main)

    if(TARGET SDL2::SDL2main)
        target_link_libraries(resl PRIVATE SDL2::SDL2main)
    endif()
    target_link_libraries(resl PRIVATE SDL2::SDL2)

    add_custom_command(
        TARGET resl
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
            ${CMAKE_CURRENT_SOURCE_DIR}/resources/PLAY.7
            $<TARGET_FILE_DIR:resl>/PLAY.7
        COMMAND ${CMAKE_COMMAND} -E copy
            ${CMAKE_CURRENT_SOURCE_DIR}/resources/GAMEOVER.7
            $<TARGET_FILE_DIR:resl>/GAMEOVER.7
        COMMAND ${CMAKE_COMMAND} -E copy
            ${CMAKE_CURRENT_SOURCE_DIR}/resources/DEMO_A
            $<TARGET_FILE_DIR:resl>/DEMO_A
    )
endif()

target_compile_options(resl PRIVATE "-fno-exceptions" "-fno-rtti")
target_include_directories(resl PRIVATE ${CMAKE_SOURCE_DIR}/src)

find_program(iwyu_path NAMES include-what-you-use iwyu)
if (NOT iwyu_path)
    message("WARNING: include-what-you-use tool was not found - will build without include validation")
else()
    set_property(TARGET resl PROPERTY CXX_INCLUDE_WHAT_YOU_USE ${iwyu_path};-Xiwyu;--no_fwd_decls;-Xiwyu;--cxx17ns)
endif()
