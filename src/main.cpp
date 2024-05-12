#include "game/init.h"

#include "game/drawing.h"
#include "game/game_data.h"
#include "game/load_game.h"
#include "game/move_trains.h"
#include "game/records.h"
#include "game/resources/train_glyph.h"
#include "graphics/color.h"
#include "graphics/drawing.h"
#include "graphics/driver.h"
#include "graphics/glyph.h"
#include "graphics/text.h"
#include "system/time.h"
#include "tasks/task.h"

#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <map>

using namespace resl;

namespace {

void testRecordsScreen(int, const char*[])
{
    showRecordsScreen();
}

void testDrawText(int, const char*[])
{
    char buf[148];
    unsigned c = 0;
    for (int y = 0; y < 10; ++y) {
        char buf[20];
        int x = 0;
        for (x = 0; x < 15 && c <= 146; ++x) {
            buf[x] = c;
            ++c;
        }
        drawText(10, 40 + y * 20, buf, Red);

        if (c > 146)
            break;
    }
}

Task implTestLoadGame()
{
    std::clock_t lastFrame = std::clock();
    TimeT lastGameTime = getTime();
    for (;;) {
        TimeT gameTime = getTime();
        std::int16_t dTime = gameTime - lastGameTime;
        lastGameTime = gameTime;

        std::clock_t curTime = std::clock();
        std::cout << "FPS: " << (CLOCKS_PER_SEC / (curTime - lastFrame)) << std::endl
                  << "dtime: " << dTime << std::endl;
        lastFrame = curTime;

        for (Train& train : trains) {
            if (!train.isFreeSlot)
                moveTrain(train, dTime);
        }
        drawWorld();

        co_await sleep(10);
    }
    co_return;
}

void testLoadGame(int argc, const char* argv[])
{
    const char* fname;
    if (argc != 3) [[unlikely]] {
        std::cout << "The game save file name is not set explicitly - loading DEMO_A file" << std::endl;
        std::cout << "You can pass the file name through the additional command line argument" << std::endl;
        fname = "DEMO_A";
    } else
        fname = argv[2];

    if (!std::filesystem::is_regular_file(fname)) [[unlikely]] {
        std::cerr << "specified file \"" << fname << "\" doesn't exist" << std::endl;
        return;
    }
    if (!std::filesystem::is_regular_file("PLAY.7")) {
        std::cerr
            << "!!!WARNING!!! The PLAY.7 file was not found!\n"
               "Please, copy PLAY.7 file from the original game folder to the application folder. "
               "Otherwise you will see a mess in the game header area"
            << std::endl;
    }

    initGameData();

    fillGameFieldBackground(0);

    char switchStatesBuf[100];
    char buf2[100];

    resetGameData();
    loadSavedGame(fname);

    addTask(implTestLoadGame());
}

Task implTestDrawTrains()
{
    static Color colors[5][2] = {
        { Blue,       DarkBlue  },
        { Red,        DarkRed   },
        { White,      Gray      },
        { Gray,       DarkGray  },
        { LightGreen, DarkGreen }
    };

    // angle - direction
    std::pair<int, int> frames[] = {
        { 0, 0 },
        { 1, 0 },
        { 2, 0 },
        { 3, 0 },
        { 4, 0 },
        { 0, 1 },
        { 1, 1 },
        { 2, 1 },
        { 3, 1 },
        { 4, 1 },
        { 0, 0 }
    };

    int curFrame = 0;
    int dFrame = 1;
    for (;;) {
        const auto [currentAngle, currentDirection] = frames[curFrame];
        drawing::filledRectangle(0, 0, 640, 480, 0xFF, DarkGreen);

        int x = 80;
        int y = 30;
        for (int i = 0; i < 15; ++i) {
            const TrainGlyph* g = &g_trainGlyphs[i][currentAngle][currentDirection];
            Color* c = colors[i % std::size(colors)];
            drawGlyph(g->glyph1, x - g->width / 2, y - g->height / 2, Black);
            drawGlyph(g->glyph2, x - g->width / 2, y - g->height / 2, c[0]);
            drawGlyph(g->glyph3, x - g->width / 2, y - g->height / 2, c[1]);
            char buf[100];
            snprintf(buf, sizeof(buf), "%d", i);
            drawText(x, y + 20, buf, Red);
            x += 80;
            if (x > 590) {
                x = 80;
                y += 60;
            }
        }

        curFrame += dFrame;
        if (curFrame == 0 || curFrame == std::size(frames) - 1)
            dFrame = -dFrame;

        co_await sleep(10);
    }
    co_return;
}

void testDrawTrains(int, const char*[])
{
    addTask(implTestDrawTrains());
}

const std::map<std::string, std::function<void(int, const char*[])>> commands = {
    { "records",        testRecordsScreen },
    { "testDrawText",   testDrawText      },
    { "testLoadGame",   testLoadGame      },
    { "testDrawTrains", testDrawTrains    }
};

int usage(const char* prog, const char* unknownArg = nullptr)
{
    if (unknownArg)
        std::cerr << "unknown argument: " << unknownArg << std::endl;

    std::cout << "usage: " << prog
              << " <mode> <arguments>\n"
                 "available modes:"
              << std::endl;
    for (auto& [cmd, _] : commands)
        std::cout << '\t' << cmd << std::endl;

    return EXIT_FAILURE;
}

Task sdlLoop()
{
    while (poll_event()) {
        graphics_update();
        co_await sleep(1);
    }
    stopScheduler();
    co_return;
}

} // namespace

int main(int argc, const char* argv[])
{
    if (argc < 2)
        return usage(*argv);

    auto iter = commands.find(argv[1]);
    if (iter == commands.end())
        return usage(argv[0], argv[1]);

    graphics_init();
    startTimer();

    addTask(sdlLoop());
    iter->second(argc, argv);

    runScheduler();

    graphics_close();

    return EXIT_SUCCESS;
}
