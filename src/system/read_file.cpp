#include "read_file.h"

#include <sys/fcntl.h>
#include <unistd.h>

#include <cstring>

#ifndef O_BINARY
#   define O_BINARY 0
#endif

namespace resl {

/* 262d:7378 - 14 bytes */
static char g_lastFileName[14];

/* 1abc:0005 */
ssize_t readFromFile(const char* fileName, void* pagePtr)
{
    int fd = open(fileName, O_BINARY | O_RDONLY);
    ssize_t nBytes = read(fd, pagePtr, 0xFFFA);
    close(fd);
    std::strcpy(g_lastFileName, fileName);
    return nBytes;
}

/* 1abc:0064 */
void readIfNotLoaded(const char* fileName, void* pagePtr)
{
    if (std::strcmp(fileName, g_lastFileName)) {
        readFromFile(fileName, pagePtr);
        std::strcpy(g_lastFileName, fileName);
    }
}

} // namespace resl
