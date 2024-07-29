#include "filesystem.h"

#include "buffer.h"

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstring>

#ifdef WIN32
#   include <windows.h>
#else
#   include <chrono>
#   include <filesystem>
#   include <glob.h>
#   include <system_error>
#endif

namespace resl {

/* 262d:7378 - 14 bytes */
static char g_lastFileName[14];

/* 1abc:0005 */
std::size_t readBinaryFile(const char* fileName, void* pagePtr)
{
    std::FILE* file = std::fopen(fileName, "rb");
    std::size_t nBytes = 0;
    if (file) {
        nBytes = std::fread(pagePtr, 1, 0xFFFA, file);
        std::fclose(file);
    }
    std::strcpy(g_lastFileName, fileName);
    return nBytes;
}

/* 1400:067f */
std::size_t readTextFile(const char* fileName)
{
    std::FILE* file = std::fopen(fileName, "r");
    std::size_t nBytes = 0;
    if (file) {
        nBytes = std::fread(g_pageBuffer, 1, 0xFFDC, file);
        std::fclose(file);
    }
    return nBytes;
}

/* 1abc:0064 */
void readIfNotLoaded(const char* fileName, void* pagePtr)
{
    if (std::strcmp(fileName, g_lastFileName)) {
        readBinaryFile(fileName, pagePtr);
        std::strcpy(g_lastFileName, fileName);
    }
}

namespace {

#ifdef WIN32

    class FileSearch {
    public:
        ~FileSearch()
        {
            close();
        }

        bool findFirst(const char* pattern)
        {
            close();
            m_handle = FindFirstFileA(pattern, &m_data);
            return m_handle != INVALID_HANDLE_VALUE;
        }

        bool findNext()
        {
            return FindNextFileA(m_handle, &m_data) != 0;
        }

        FileInfo lastSearchResult()
        {
            FileInfo result = {};
            result.fileName = m_data.cFileName;

            FILETIME localTime;
            SYSTEMTIME st;
            if (FileTimeToLocalFileTime(&m_data.ftCreationTime, &localTime) &&
                FileTimeToSystemTime(&localTime, &st)) {

                // Date and time format:
                // https://www.stanislavs.org/helppc/file_attributes.html
                assert(st.wDay >= 1 && st.wDay <= 31);
                assert(st.wMonth >= 1 && st.wMonth <= 12);
                result.fileDate =
                    st.wDay | (st.wMonth << 5) | ((st.wYear - 1980) << 9);

                assert(st.wSecond < 60);
                assert(st.wMinute < 60);
                assert(st.wHour < 24);
                result.fileTime =
                    (st.wSecond * 2) | (st.wMinute << 5) | (st.wHour << 11);
            }
            return result;
        }

    private:
        void close()
        {
            if (m_handle != INVALID_HANDLE_VALUE) {
                FindClose(m_handle);
                m_handle = INVALID_HANDLE_VALUE;
            }
        }

        WIN32_FIND_DATA m_data;
        HANDLE m_handle = INVALID_HANDLE_VALUE;
    };

#else // WIN32

    class FileSearch {
    public:
        ~FileSearch()
        {
            close();
        }

        bool findFirst(const char* pattern)
        {
            close();
            m_glob = {};
            m_curPath = 0;
            return glob(pattern, 0, nullptr, &m_glob) == 0;
        }

        bool findNext()
        {
            ++m_curPath;
            return m_curPath < m_glob.gl_pathc;
        }

        FileInfo lastSearchResult()
        {
            using namespace std::chrono;

            assert(m_curPath < m_glob.gl_pathc);
            FileInfo result;

            result.fileName = m_glob.gl_pathv[m_curPath];

            const std::filesystem::path p(m_glob.gl_pathv[m_curPath]);
            std::error_code ec;
            std::filesystem::file_time_type fileTime =
                std::filesystem::last_write_time(p, ec);
            if (ec == std::error_code()) [[likely]] {
                system_clock::time_point sysTime = time_point_cast<system_clock::duration>(
                    fileTime - std::filesystem::file_time_type::clock::now() + system_clock::now());

                // Date and time format:
                // https://www.stanislavs.org/helppc/file_attributes.html
                auto tpDays = std::chrono::floor<days>(sysTime);
                year_month_day date(tpDays);
                assert(date.ok());
                const unsigned day = static_cast<unsigned>(date.day());
                const unsigned month = static_cast<unsigned>(date.month());
                const int year = std::max(static_cast<int>(date.year()), 1980);
                assert(day >= 1 && day <= 31);
                assert(month >= 1 && month <= 12);
                result.fileDate = day | (month << 5) | ((year - 1980) << 9);

                hh_mm_ss time(std::chrono::floor<seconds>(sysTime - tpDays));
                assert(time.seconds().count() < 60);
                assert(time.minutes().count() < 60);
                assert(time.hours().count() < 24);
                result.fileTime = (time.seconds().count() * 2) |
                    (time.minutes().count() << 5) |
                    (time.hours().count() << 11);
            }
            return result;
        }

    private:
        void close()
        {
            globfree(&m_glob);
        }

        glob_t m_glob;
        std::size_t m_curPath;
    };

#endif // platform specific

    FileSearch g_search;

} // namespace

/* 12b1:0006 */
int findFirst(const char* pattern, [[maybe_unused]] std::uint8_t attrs)
{
    // The original implementation is based on DOS 21h (4E) API call:
    //      https://www.stanislavs.org/helppc/int_21-4e.html

    assert(attrs == 0); // other modes are not supported
    return g_search.findFirst(pattern) ? 0 : 1;
}

/* 12b1:001d */
int findNext()
{
    // The original implementation is based on DOS 21h (4F) API call:
    //      https://www.stanislavs.org/helppc/int_21-4f.html
    return g_search.findNext() ? 0 : 1;
}

/* 1000:12a8 */
FileInfo lastSearchResult()
{
    // The original implementation is based on DOS 21h (2F) API call:
    //      https://www.stanislavs.org/helppc/int_21-2f.html
    return g_search.lastSearchResult();
}

} // namespace resl
