cmake_minimum_required(VERSION 3.20)

project(reSL)

if (NOT EMSCRIPTEN)
    option(USE_SYSTEM_SDL2 "use SDL2 library from the system" OFF)
endif()

set (APP_VERSION "1.1.1")

option(USE_IWYU "enable include-what-you-use tool" OFF)
option(BUILD_NUMBER "build version number" "1")

file(GLOB_RECURSE headers "*.h")

set(sources
    src/game/demo.cpp
    src/game/drawing.cpp
    src/game/entrance.cpp
    src/game/field_tile_grid_overlay.cpp
    src/game/header.cpp
    src/game/impasse.cpp
    src/game/init.cpp
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
    src/system/file.cpp
    src/system/filesystem.cpp
    src/system/keyboard.cpp
    src/system/random.cpp
    src/system/sound.cpp
    src/system/time.cpp
    src/tasks/scheduler.cpp
    src/tasks/task.cpp
    src/ui/components/button.cpp
    src/ui/components/close_button.cpp
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
    resources/demo_b
    resources/demo_c
    resources/demo_d
    resources/demo_e
    resources/demo_f
    resources/demo_g
    resources/demo_h
    resources/demo_i
    resources/demo_j
    resources/demo_k
    resources/demo_l
    resources/demo_m
    resources/demo_n
    resources/demo_o
    resources/demo_p
    resources/demo_q
    resources/demo_r
    resources/demo_s
    resources/demo_t
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

if (ANDROID)
    add_library(resl SHARED ${sources} ${headers})

    # https://developer.android.com/guide/practices/page-sizes#cmake
    target_link_options(resl PRIVATE "-Wl,-z,max-page-size=16384")
else()
    add_executable(resl ${sources} ${headers})
endif()

if(IOS)
    set_target_properties(resl PROPERTIES
        BUNDLE TRUE
        MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/ios/config/iOSBundleInfo.plist.in
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_BUNDLE_VERSION ${BUILD_NUMBER}
        MACOSX_BUNDLE_SHORT_VERSION_STRING ${APP_VERSION}
        MACOSX_BUNDLE_LONG_VERSION_STRING "${APP_VERSION} (${BUILD_NUMBER})"
        MACOSX_BUNDLE_GUI_IDENTIFIER com.konovalovaleks.resl
        #MACOSX_BUNDLE_ICON_FILE
    )
endif()

set_property(TARGET resl PROPERTY CXX_STANDARD 20)

include(GNUInstallDirs)

if (EMSCRIPTEN)
    target_compile_options(resl PRIVATE
        "--use-port=sdl2"
        "--use-port=sdl2_image:formats=png")
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
    if (${USE_SYSTEM_SDL2})
        find_package(SDL2 CONFIG REQUIRED)
        find_package(SDL2_image REQUIRED)
        target_link_libraries(resl PRIVATE SDL2::SDL2 SDL2_image::SDL2_image)
    else()
        include(FetchContent)
        set(FETCHCONTENT_QUIET FALSE)
        if (ANDROID)
            set(FETCHCONTENT_BASE_DIR "${CMAKE_SOURCE_DIR}/.fetch_content")
        endif()

        set(BUILD_SHARED_LIBS OFF)

        FetchContent_Declare(
            SDL2
            GIT_REPOSITORY  https://github.com/libsdl-org/SDL
            GIT_TAG         release-2.30.10
            GIT_PROGRESS    TRUE
        )
        if (NOT ANDROID)
            set(SDL_HAPTIC OFF)
            set(SDL_HIDAPI OFF)
            set(SDL_TIMERS OFF)
        endif()
        set(SDL_CPUINFO OFF)
        set(SDL_FILE OFF)
        set(SDL_FILESYSTEM OFF)
        set(SDL_LOCALE OFF)
        set(SDL_MISC OFF)
        set(SDL_POWER OFF)
        set(SDL_SENSOR OFF)

        FetchContent_Declare(
            SDL2_image
            GIT_REPOSITORY  https://github.com/libsdl-org/SDL_image
            GIT_TAG         release-2.8.2
            GIT_PROGRESS    TRUE
        )

        set(SDL2IMAGE_PNG ON)

        set(SDL2IMAGE_JPG_SAVE OFF)
        set(SDL2IMAGE_PNG_SAVE OFF)
        set(SDL2IMAGE_AVIF OFF)
        set(SDL2IMAGE_BMP OFF)
        set(SDL2IMAGE_GIF OFF)
        set(SDL2IMAGE_JPG OFF)
        set(SDL2IMAGE_JXL OFF)
        set(SDL2IMAGE_LBM OFF)
        set(SDL2IMAGE_PCX OFF)
        set(SDL2IMAGE_PNM OFF)
        set(SDL2IMAGE_QOI OFF)
        set(SDL2IMAGE_SVG OFF)
        set(SDL2IMAGE_TGA OFF)
        set(SDL2IMAGE_TIF OFF)
        set(SDL2IMAGE_WEBP OFF)
        set(SDL2IMAGE_XCF OFF)
        set(SDL2IMAGE_XPM OFF)
        set(SDL2IMAGE_XV OFF)

        set(SDL2IMAGE_DEPS_SHARED OFF)
        set(SDL2IMAGE_TESTS OFF)

        FetchContent_MakeAvailable(SDL2 SDL2_IMAGE)

        set(SDL_LIBRARIES ${SDL_LIBRARIES} SDL2main SDL2-static SDL2_image::SDL2_image-static)
        target_link_libraries(resl PRIVATE ${SDL_LIBRARIES})
    endif()

    if (ANDROID)
        set(RESOURCE_DST_DIR ${CMAKE_SOURCE_DIR}/android-project/app/src/main/assets)
    else()
        set(RESOURCE_DST_DIR $<TARGET_FILE_DIR:resl>)
    endif()

    if (NOT IOS)
        install(TARGETS resl RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
        install(FILES
            ${resources}
            DESTINATION ${CMAKE_INSTALL_BINDIR}
        )
    endif()

    foreach (path ${resources})
        get_filename_component(fname ${path} NAME)
        add_custom_command(
            TARGET resl
            POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy
                ${CMAKE_CURRENT_SOURCE_DIR}/${path}
                ${RESOURCE_DST_DIR}/${fname}

            COMMAND ${CMAKE_COMMAND} -E copy
                ${CMAKE_CURRENT_SOURCE_DIR}/resources/reSL/build_rail_icon.png
                ${RESOURCE_DST_DIR}/build_rail_icon.png
            COMMAND ${CMAKE_COMMAND} -E copy
                ${CMAKE_CURRENT_SOURCE_DIR}/resources/reSL/call_server_icon.png
                ${RESOURCE_DST_DIR}/call_server_icon.png
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

if (${USE_IWYU})
    find_program(iwyu_path NAMES include-what-you-use iwyu REQUIRED)
    set_property(TARGET resl PROPERTY CXX_INCLUDE_WHAT_YOU_USE ${iwyu_path};-Xiwyu;--no_fwd_decls;-Xiwyu;--cxx17ns)
endif()
