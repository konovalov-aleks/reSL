#include "exit.h"

#include "sound.h"

#include <cstdlib>
#include <iostream>

namespace resl {

/* 15ab:024d */
void exitWithMessage(const char* msg)
{
    // The original game makes a lot of different cleanup routines
    // (e.g. restore the original interruption vector,
    //  restore the original mouse handler, etc)
    // This is all irrelevant on modern systems.

    nosound();
    std::cout << msg << std::endl;
    std::exit(EXIT_SUCCESS);
}

} // namespace resl
