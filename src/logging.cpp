#include "logging.h"

#include <cstdarg>
#include <cstdio>

void log_message(const char* log_level, const char* ansi_escape_code, const char* format, ...)
{
    std::printf("%s[%s] ", ansi_escape_code, log_level);

    std::va_list args;
    va_start(args, format);
    std::vprintf(format, args);
    va_end(args);

    std::printf("%s\n", ANSI_ESCAPE_CODE_RESET);
}

void log_message(
    const char* log_level,
    const char* ansi_escape_code,
    const char* file,
    int line,
    const char* format,
    ...
)
{
    std::fprintf(stderr, "%s[%s] ", ansi_escape_code, log_level);

    std::va_list args;
    va_start(args, format);
    std::vfprintf(stderr, format, args);
    va_end(args);

    std::fprintf(stderr, " | %s (%d)%s\n", file, line, ANSI_ESCAPE_CODE_RESET);
}
