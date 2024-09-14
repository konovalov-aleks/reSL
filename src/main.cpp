#include "game/init.h"
#include "game/main_loop.h"
#include "game/mouse/mouse.h"
#include "game/player_name.h"
#include "game/train.h"
#include "graphics/vga.h"
#include "system/active_sleep.h"
#include "system/filesystem.h"
#include "system/keyboard.h"
#include "system/time.h"
#include "tasks/task.h"
#include <system/driver/driver.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <iostream>

using namespace resl;

namespace {

// Helper task for handling SDL events.
Task sdlLoop()
{
    for (;;) {
        Driver::instance().vga().flush();
        co_await sleep(2);
    }
}

int usage(int /* argc */, char* argv[], int unknownArg = -1)
{
    if (unknownArg != -1)
        std::cerr << "Unknown command line argument \"" << argv[unknownArg] << "\"\n"
                  << std::endl;
    std::cerr << "Usage: " << argv[0]
              << " [--debug-graphics] [--windowed] [playerName] [S<random seed>]\n"
                 "\n"
                 "The following options are available:\n"
                 "  --debug-graphics  enable debug video mode. In this mode, you will see the entire content of the video memory including invisible areas\n"
                 "  --windowed        run the game in windowed mode, do not expand to full screen\n"
                 "  --help            show this help\n"
                 "\n"
              << std::endl;
    return EXIT_FAILURE;
}

} // namespace

namespace resl {

/* 1c75:027c */
static void initTasks(/* void* taskStacksMemory */)
{
    /*
       The original game uses a platform-specific stack-based coroutine
       implementation. But reSL uses a portable approach based on C++20 coroutines.
       So, this implementation of the function is different (but logically the same).
    */

    addTask(taskSpawnTrains());
    addTask(taskMouseEventHandling());
    g_taskGameMainLoop = addTask(taskGameMainLoop());

    runScheduler();
}

// main function of the original game
/* 15ab:0018 */
int main(int argc, char* argv[])
{
    const unsigned seed = static_cast<unsigned>(std::time(nullptr));
    std::srand(seed);
    if (argc == 3 && argv[2][0] == 'S') {
        std::srand(static_cast<unsigned>(argv[2][1]));
        std::printf("Seed = %d\n", static_cast<int>(argv[2][1]));
    }
    if (argc > 1)
        std::strcpy(g_playerName, argv[1]);

    /*
        The original game performs initialization here:
            15ab:0096 > mouse driver initialization
            15ab:00b2 > allocates memory for task stacks
            15ab:00d5 > allocates memory for g_pageBuffer (262d:21d8)
            15ab:00f3 > sets the video mode (VGA 640x350, 16 colors)
            15ab:00fe > sets the number of bytes in a VGA scanline
    */

    vga::setVideoModeR0W2();
    initGameData();

    /*
        The original game sets interruption handlers here:
            15ab:0136 > DIV/0 handler
            15ab:015c > timer handler
    */
    initTimer();
    calibrateActiveSleep();

    Driver::instance().setKeyboardHandler(&keyboardInterruptionHandler);
    Driver::instance().mouse().setHandler(&handleMouseInput);

    initTasks(/* taskStacksMemory */);

    return EXIT_SUCCESS;
}

} // namespace resl

int main(int argc, char* argv[])
{
    bool debugGraphics = false;
    bool fullscreen = true;

    char* origGameArgv[3];
    origGameArgv[0] = argv[1];
    int origGameArgc = 1;

    for (int i = 1; i < argc; ++i) {
        if (!std::strcmp(argv[i], "--debug-graphics"))
            debugGraphics = true;
        if (!std::strcmp(argv[i], "--windowed"))
            fullscreen = false;
        else if (!std::strcmp(argv[i], "--help"))
            return usage(argc, argv);
        else if (argv[i][0] != '-' && origGameArgc < 3)
            origGameArgv[origGameArgc++] = argv[i];
        else
            return usage(argc, argv, i);
    }

    initFS();

    if (debugGraphics)
        Driver::instance().vga().setDebugMode(true);
    if (fullscreen)
        Driver::instance().vga().setFullscreenMode(true);

    addTask(sdlLoop());
    return resl::main(origGameArgc, origGameArgv);
}
