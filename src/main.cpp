#include "game/init.h"

#include "game/drawing.h"
#include "game/game_data.h"
#include "game/header.h"
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
#include <cstring>
#include <filesystem>
#include <functional>
#include <iostream>
#include <map>

using namespace resl;

namespace {

void recordsScreenDemo(int, const char*[])
{
    showRecordsScreen();
}

void drawTextDemo(int, const char*[])
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

void loadGame(const char* fname)
{
    if (!fname) {
        std::cout << "The game save file name is not set explicitly - loading DEMO_A file" << std::endl;
        std::cout << "You can pass the file name through the '--file' command line argument" << std::endl;
        fname = "DEMO_A";
    }

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

    char switchStatesBuf[100];
    char buf2[100];

    resetGameData();
    loadSavedGame(fname);

    drawWorld();
    fillGameFieldBackground(350);
    drawFieldBackground(350);

    drawing::setVideoModeR0W1();
    drawing::copyRectangle(0, 0, 0, 350, 80, 350);
    drawing::setVideoModeR0W2();

    addTask(taskHeaderFieldAnimation());
    addTask(taskMoveAndRedrawTrains());
}

Task implDrawTrainsDemo()
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

void drawTrainsDemo(int, const char*[])
{
    addTask(implDrawTrainsDemo());
}

const std::map<std::string, std::function<void(int, const char*[])>> commands = {
    { "records",     recordsScreenDemo },
    { "draw_text",   drawTextDemo      },
    { "draw_trains", drawTrainsDemo    }
};

Task sdlLoop()
{
    while (poll_event()) {
        graphics_update();
        co_await sleep(1);
    }
    stopScheduler();
    co_return;
}

int usage(int argc, const char* argv[], int unknownArg = -1)
{
    if (unknownArg != -1)
        std::cerr << "Unknown command line argument \"" << argv[unknownArg] << "\"\n"
                  << std::endl;
    std::cerr << "Usage: " << argv[0] << " <options>\n"
                                         "\n"
                                         "Available options:\n"
                                         "  --file <fileName> choose the game save file to load\n"
                                         "  --demo <demoName> run the demo with a specified name. Available demos: ";
    bool first = true;
    for (const auto& [name, _] : commands) {
        if (first)
            first = false;
        else
            std::cerr << ", ";
        std::cerr << name;
    }
    std::cerr << "\n"
                 "  --debug-graphics enable debug video mode. In this mode, you will see the entire content of the video memory including invisible areas\n"
                 "  --help show this help"
              << std::endl;
    return EXIT_FAILURE;
}

} // namespace

int main(int argc, const char* argv[])
{
    bool debugGraphics = false;
    const char* demo = nullptr;
    const char* file = nullptr;

    for (int i = 1; i < argc; ++i) {
        if (!std::strcmp(argv[i], "--debug-graphics"))
            debugGraphics = true;
        else if (!std::strcmp(argv[i], "--demo")) {
            if (i == argc - 1) {
                std::cerr << "The '--demo' argument expects a value" << std::endl;
                return EXIT_FAILURE;
            }
            demo = argv[++i];
        } else if (!std::strcmp(argv[i], "--file")) {
            if (i == argc - 1) {
                std::cerr << "The '--file' argument expects a value" << std::endl;
                return EXIT_FAILURE;
            }
            file = argv[++i];
        } else if (!std::strcmp(argv[i], "--help"))
            return usage(argc, argv);
        else
            return usage(argc, argv, i);
    }

    graphics_init(debugGraphics);

    if (demo) {
        if (file)
            std::cerr << "The '--file' attribute was ignored" << std::endl;
        auto iter = commands.find(demo);
        if (iter == commands.end()) {
            std::cerr << "Invalid demo name is specified \"" << demo << "\"\n"
                      << std::endl;
            return usage(argc, argv);
        }
        iter->second(argc, argv);
    } else
        loadGame(file);

    addTask(sdlLoop());

    startTimer();
    runScheduler();

    graphics_close();

    return EXIT_SUCCESS;
}
