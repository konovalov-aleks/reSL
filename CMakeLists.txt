cmake_minimum_required(VERSION 3.5)

project(reSL)

file(GLOB_RECURSE HEADERS "*.h")

set(SOURCES
    src/game/draw_header.cpp
    src/game/draw_impasse.cpp
    src/game/drawing.cpp
    src/game/game_data.cpp
    src/game/header.cpp
    src/game/init.cpp
    src/game/io_status.cpp
    src/game/load_game.cpp
    src/game/move_trains.cpp
    src/game/records.cpp
    src/game/resources/carriage_bias.cpp
    src/game/resources/dispatcher_glyph.cpp
    src/game/resources/entrance.cpp
    src/game/resources/impasse_glyph.cpp
    src/game/resources/movement_paths.cpp
    src/game/resources/rail_glyph.cpp
    src/game/resources/rail_info.cpp
    src/game/resources/rail_type_meta.cpp
    src/game/resources/s4arr.cpp
    src/game/resources/semaphore_glyph.cpp
    src/game/resources/semaphore_glyph_bias.cpp
    src/game/resources/static_object_glyph.cpp
    src/game/resources/train_finished_exclamation_glyph.cpp
    src/game/resources/train_glyph.cpp
    src/game/semaphore.cpp
    src/game/switch.cpp
    src/game/types/entrance.cpp
    src/graphics/drawing.cpp
    src/graphics/glyph.cpp
    src/graphics/text.cpp
    src/main.cpp
    src/system/active_sleep.cpp
    src/system/buffer.cpp
    src/system/driver/sdl/driver.cpp
    src/system/driver/sdl/video.cpp
    src/system/random.cpp
    src/system/read_file.cpp
    src/system/time.cpp
    src/tasks/scheduler.cpp
    src/tasks/task.cpp
)

find_package(SDL2 REQUIRED CONFIG REQUIRED COMPONENTS SDL2)
find_package(SDL2 REQUIRED CONFIG COMPONENTS SDL2main)

add_executable(resl ${SOURCES} ${HEADERS})

set_property(TARGET resl PROPERTY CXX_STANDARD 20)

if(TARGET SDL2::SDL2main)
    target_link_libraries(resl PRIVATE SDL2::SDL2main)
endif()
target_link_libraries(resl PRIVATE SDL2::SDL2)

target_include_directories(resl PRIVATE ${CMAKE_SOURCE_DIR}/src)

add_custom_command(
    TARGET resl
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_CURRENT_SOURCE_DIR}/resources/PLAY.7
        $<TARGET_FILE_DIR:resl>/PLAY.7
    COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_CURRENT_SOURCE_DIR}/resources/DEMO_A
        $<TARGET_FILE_DIR:resl>/DEMO_A
)
