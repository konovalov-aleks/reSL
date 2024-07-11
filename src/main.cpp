#include "game/init.h"

#include "game/keyboard.h"
#include "game/main_loop.h"
#include "game/mouse/mouse.h"
#include "game/records.h"
#include "game/resources/train_glyph.h"
#include "game/train.h"
#include "graphics/color.h"
#include "graphics/drawing.h"
#include "graphics/glyph.h"
#include "graphics/text.h"
#include "system/time.h"
#include "tasks/task.h"
#include <system/driver/driver.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <filesystem>
#include <functional>
#include <iostream>
#include <iterator>
#include <map>
#include <string>
#include <utility>

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
        drawText(10, 40 + y * 20, buf, Color::Red);

        if (c > 146)
            break;
    }
}

void startGame()
{
    if (!std::filesystem::is_regular_file("PLAY.7")) {
        std::cerr
            << "!!!WARNING!!! The PLAY.7 file was not found!\n"
               "Please, copy PLAY.7 file from the original game folder to the application folder. "
               "Otherwise you will see a mess in the game header area"
            << std::endl;
    }

    initGameData();

    Driver::instance().setMouseHandler(&handleMouseInput);
    Driver::instance().setKeyboardHandler(&keyboardInteruptionHandler);

    addTask(taskMouseEventHandling());
    g_taskGameMainLoop = addTask(taskGameMainLoop());
    addTask(taskSpawnTrains());
}

Task implDrawTrainsDemo()
{
    static Color colors[5][2] = {
        { Color::Blue,       Color::DarkBlue  },
        { Color::Red,        Color::DarkRed   },
        { Color::White,      Color::Gray      },
        { Color::Gray,       Color::DarkGray  },
        { Color::LightGreen, Color::DarkGreen }
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
        graphics::filledRectangle(0, 0, 640, 480, 0xFF, DarkGreen);

        int x = 80;
        int y = 30;
        for (int i = 0; i < 15; ++i) {
            const TrainGlyph* g = &g_trainGlyphs[i][currentAngle][currentDirection];
            Color* c = colors[i % std::size(colors)];
            drawGlyph(g->glyph1, x - g->width / 2, y - g->height / 2, Color::Black);
            drawGlyph(g->glyph2, x - g->width / 2, y - g->height / 2, c[0]);
            drawGlyph(g->glyph3, x - g->width / 2, y - g->height / 2, c[1]);
            char buf[100];
            std::snprintf(buf, sizeof(buf), "%d", i);
            drawText(x, y + 20, buf, Red);
            x += 80;
            if (x > 590) {
                x = 80;
                y += 60;
            }
        }

        curFrame += dFrame;

        if (curFrame < 4)
            Driver::instance().audio().startSound(400 + 100 * (3 - curFrame));
        else
            Driver::instance().audio().stopSound();

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
    for (;;) {
        Driver::instance().pollEvent();
        co_await sleep(2);
    }
}

int usage(int argc, const char* argv[], int unknownArg = -1)
{
    if (unknownArg != -1)
        std::cerr << "Unknown command line argument \"" << argv[unknownArg] << "\"\n"
                  << std::endl;
    std::cerr << "Usage: " << argv[0] << " <options>\n"
                                         "\n"
                                         "Available options:\n"
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

    std::srand(static_cast<unsigned>(std::time(nullptr)));

    for (int i = 1; i < argc; ++i) {
        if (!std::strcmp(argv[i], "--debug-graphics"))
            debugGraphics = true;
        else if (!std::strcmp(argv[i], "--demo")) {
            if (i == argc - 1) {
                std::cerr << "The '--demo' argument expects a value" << std::endl;
                return EXIT_FAILURE;
            }
            demo = argv[++i];
        } else if (!std::strcmp(argv[i], "--help"))
            return usage(argc, argv);
        else
            return usage(argc, argv, i);
    }

    if (debugGraphics)
        Driver::instance().vga().setDebugMode(true);

    initTimer();

    if (demo) {
        auto iter = commands.find(demo);
        if (iter == commands.end()) {
            std::cerr << "Invalid demo name is specified \"" << demo << "\"\n"
                      << std::endl;
            return usage(argc, argv);
        }
        iter->second(argc, argv);
    } else
        startGame();

    addTask(sdlLoop());
    runScheduler();

    return EXIT_SUCCESS;
}
