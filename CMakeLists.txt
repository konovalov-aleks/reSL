cmake_minimum_required(VERSION 3.20)

project(reSL)

file(GLOB_RECURSE headers "*.h")

set(sources
    src/game/demo.cpp
    src/game/drawing.cpp
    src/game/entrance.cpp
    src/game/field_tile_grid_overlay.cpp
    src/game/header.cpp
    src/game/impasse.cpp
    src/game/init.cpp
    src/game/io_status.cpp
    src/game/main_loop.cpp
    src/game/melody.cpp
    src/game/mouse/construction_mode.cpp
    src/game/mouse/management_mode.cpp
    src/game/mouse/mouse.cpp
    src/game/mouse/state.cpp
    src/game/move_trains.cpp
    src/game/player_name.cpp
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
    src/game/resources/rail_connection_rule.cpp
    src/game/resources/rail_glyph.cpp
    src/game/resources/rail_type_meta.cpp
    src/game/resources/semaphore_glyph.cpp
    src/game/resources/semaphore_glyph_bias.cpp
    src/game/resources/small_font.cpp
    src/game/resources/static_object_glyph.cpp
    src/game/resources/text_glyphs.cpp
    src/game/resources/train_finished_exclamation_glyph.cpp
    src/game/resources/train_glyph.cpp
    src/game/resources/train_specification.cpp
    src/game/road_construction.cpp
    src/game/savefile/load_game.cpp
    src/game/savefile/save_game.cpp
    src/game/semaphore.cpp
    src/game/static_object.cpp
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
    src/system/driver/sdl/mouse.cpp
    src/system/driver/sdl/texture.cpp
    src/system/driver/sdl/touch_handler.cpp
    src/system/driver/sdl/video.cpp
    src/system/exit.cpp
    src/system/filesystem.cpp
    src/system/keyboard.cpp
    src/system/random.cpp
    src/system/sound.cpp
    src/system/time.cpp
    src/tasks/scheduler.cpp
    src/tasks/task.cpp
    src/ui/components/button.cpp
    src/ui/components/dialog.cpp
    src/ui/components/draw_header.cpp
    src/ui/components/menu_button.cpp
    src/ui/components/status_bar.cpp
    src/ui/game_over.cpp
    src/ui/loading_screen.cpp
    src/ui/main_menu.cpp
    src/ui/manual.cpp
)

set(resources
    resources/play.7
    resources/GAMEOVER.7
    resources/demo_a
    resources/captions.7
    resources/poster.7
    resources/RULES.TXT
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

add_executable(resl ${sources} ${headers})

set_property(TARGET resl PROPERTY CXX_STANDARD 20)

include(GNUInstallDirs)

if (EMSCRIPTEN)
    target_compile_options(resl PRIVATE "-sUSE_SDL=2" "-msimd128")
    target_link_options(resl PRIVATE
        "-sASYNCIFY"
        "--use-port=sdl2"
        "--use-port=sdl2_image:formats=png"
        "-sINVOKE_RUN=0"
        "-sEXIT_RUNTIME=1"
        "-sMODULARIZE=1"
        "-sEXPORT_NAME=\"createModule\""
        "-sEXPORTED_RUNTIME_METHODS=[\"callMain\",\"addOnExit\",\"JSEvents\"]"
        "-lidbfs.js")

    list(TRANSFORM resources PREPEND "${CMAKE_CURRENT_SOURCE_DIR}/")
    file (GLOB reSL LIST_DIRECTORIES false ${CMAKE_CURRENT_SOURCE_DIR}/resources/reSL/*)
    file (GLOB extra LIST_DIRECTORIES false ${CMAKE_CURRENT_SOURCE_DIR}/resources/extra/*)
    list (APPEND resources ${reSL} ${extra})

    foreach(path ${resources})
        get_filename_component(fname ${path} NAME)
        MESSAGE("Embed the file: ${path}")
        target_link_options(resl PRIVATE "--embed-file=${path}@${fname}")
    endforeach()
else()
    find_package(SDL2 CONFIG REQUIRED)
    find_package(SDL2_image REQUIRED)
    target_link_libraries(resl PRIVATE SDL2::SDL2 SDL2_image::SDL2_image)

    install(TARGETS resl RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
    install(FILES
        ${resources}
        DESTINATION ${CMAKE_INSTALL_BINDIR}
    )

    foreach (path ${resources})
        get_filename_component(fname ${path} NAME)
        add_custom_command(
            TARGET resl
            POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy
                ${CMAKE_CURRENT_SOURCE_DIR}/${path}
                $<TARGET_FILE_DIR:resl>/${fname}

        COMMAND ${CMAKE_COMMAND} -E copy
            ${CMAKE_CURRENT_SOURCE_DIR}/resources/reSL/build_rail_icon.png
            $<TARGET_FILE_DIR:resl>/build_rail_icon.png
        )
    endforeach()

    if ("${CMAKE_SYSTEM_NAME}" STREQUAL "Windows")
        set(RUNTIME_DLL_SET $<BOOL:$<TARGET_RUNTIME_DLLS:resl>>)
        set(COPY_COMMAND
            ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_RUNTIME_DLLS:resl>
            $<TARGET_FILE_DIR:resl>
        )
        add_custom_command(
            TARGET resl
            POST_BUILD
            COMMAND "$<${RUNTIME_DLL_SET}:${COPY_COMMAND}>"
            COMMAND_EXPAND_LISTS
            COMMENT "Copying runtime dependencies (DLLs)"
        )
    endif()
endif()

if (MSVC)
    target_compile_options(resl PRIVATE "/D_CRT_SECURE_NO_WARNINGS" "/GR-" "/EHsc" "/D_HAS_EXCEPTIONS=0")
else()
    target_compile_options(resl PRIVATE "-fno-exceptions" "-fno-rtti" "-Wextra" "-Wall")
endif()

target_include_directories(resl PRIVATE ${CMAKE_SOURCE_DIR}/src)

find_program(iwyu_path NAMES include-what-you-use iwyu)
if (NOT iwyu_path)
    message("WARNING: include-what-you-use tool was not found - will build without include validation")
else()
    set_property(TARGET resl PROPERTY CXX_INCLUDE_WHAT_YOU_USE ${iwyu_path};-Xiwyu;--no_fwd_decls;-Xiwyu;--cxx17ns)
endif()
