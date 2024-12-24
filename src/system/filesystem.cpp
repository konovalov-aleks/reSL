#include "filesystem.h"

#include "buffer.h"
#include "file.h"

#include <algorithm>
#include <cassert>
#include <cstdint>

#ifdef WIN32
#   include <windows.h>
#else
#   include <chrono>
#   include <filesystem>
#   include <string>
#   include <system_error>
#endif

#ifdef __EMSCRIPTEN__
#   include <emscripten.h>

#   include <tuple>
#endif // __EMSCRIPTEN__

#ifdef ANDROID
#   include <SDL_system.h>
#endif // ANDROID

namespace resl {

/* 262d:7378 - 14 bytes */
static char g_lastFileName[14];

void initFS()
{
#ifdef __EMSCRIPTEN__
    static_assert(
        std::tuple(__EMSCRIPTEN_major__, __EMSCRIPTEN_minor__, __EMSCRIPTEN_tiny__) >=
            std::tuple(3, 1, 61),
        "emscripten older than 3.1.61 doesn't support autoPersist flag "
        "=> this code will work incorrectly");

    EM_ASM({
        var path = UTF8ToString($0);
        FS.mkdir(path);
        FS.mount(IDBFS, {
            autoPersist: true
        }, path);
        FS.syncfs(true, (err) => {
            console.error(`FS.syncfs failed: ${err}`);
        }); }, g_persistentFolder);
#endif // __EMSCRIPTEN__
}

/* 1abc:0005 */
std::size_t readBinaryFile(const char* fileName, void* pagePtr)
{
    std::strcpy(g_lastFileName, fileName);
    return File(fileName, "rb").read(pagePtr, 0xFFFA);
}

/* 1400:067f */
std::size_t readTextFile(const char* fileName)
{
    // The original game uses "text" mode here, which can perform
    // system-dependent transformations.
    // E.g: in this mode, DOS automatically replaces line endings with "\r\n".
    // But reSL is portable, so we read files as is (in fact, this functions
    // is only used to read RULES.TXT, which is already uses \r\n line endings)
    return File(fileName, "rb").read(g_pageBuffer, 0xFFDC);
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

#else // !WIN32

    class FileSearch {
    public:
        bool findFirst(const char* pattern)
        {
        #ifdef ANDROID
            std::filesystem::path p(pattern);
            if (!p.is_absolute())
                p = SDL_AndroidGetInternalStoragePath() / p;
        #else
            std::filesystem::path p(std::filesystem::absolute(pattern));
        #endif
            m_pattern = p.filename().string();
            std::error_code ec;
            m_dirIter = std::filesystem::directory_iterator(p.parent_path(), ec);
            return ec == std::error_code() && findNextEntry();
        }

        bool findNext()
        {
            assert(std::filesystem::begin(m_dirIter) != std::filesystem::end(m_dirIter));
            ++m_dirIter;
            return std::filesystem::begin(m_dirIter) != std::filesystem::end(m_dirIter)
                && findNextEntry();
        }

        FileInfo lastSearchResult()
        {
            using namespace std::chrono;

            assert(std::filesystem::begin(m_dirIter) != std::filesystem::end(m_dirIter));
            FileInfo result;

            m_curFileName = m_dirIter->path().filename().string();
            result.fileName = m_curFileName.c_str();

            const std::filesystem::path& p = m_dirIter->path();
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

        bool findNextEntry()
        {
            for (const std::filesystem::directory_entry& e : m_dirIter) {
                if (!e.is_regular_file())
                    continue;
                if (matchPattern(e.path().filename()))
                    return true;
            }
            return false;
        }


        bool matchPattern(const std::string& p)
        {
            // The simplified glob syntax is enough for reSL
            // (only supports '?' placeholder and constant symbols)
            if (p.size() != m_pattern.size())
                return false;

            for (std::size_t i = 0; i < m_pattern.size(); ++i) {
                if (m_pattern[i] != '?' && m_pattern[i] != p[i])
                    return false;
            }
            return true;
        }

        std::filesystem::directory_iterator m_dirIter;
        std::string m_pattern;
        std::string m_curFileName;
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
