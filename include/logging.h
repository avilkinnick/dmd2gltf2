#pragma once

#define ANSI_ESCAPE_CODE_RESET "\033[0m"
#define ANSI_ESCAPE_CODE_RED "\033[31m"
#define ANSI_ESCAPE_CODE_GREEN "\033[32m"
#define ANSI_ESCAPE_CODE_YELLOW "\033[33m"
#define ANSI_ESCAPE_CODE_BLUE "\033[34m"

void log_message(const char* log_level, const char* ansi_escape_code, const char* format, ...);

void log_message(
    const char* log_level,
    const char* ansi_escape_code,
    const char* file,
    int line,
    const char* format,
    ...
);

#define LOG_FATAL(format, ...) \
    log_message("FATAL", ANSI_ESCAPE_CODE_RED, __FILE__, __LINE__, format, ##__VA_ARGS__)

#define LOG_ERROR(format, ...) \
    log_message("ERROR", ANSI_ESCAPE_CODE_RED, __FILE__, __LINE__, format, ##__VA_ARGS__)

#define LOG_WARN(format, ...) \
    log_message("WARN", ANSI_ESCAPE_CODE_YELLOW, __FILE__, __LINE__, format, ##__VA_ARGS__)

#define LOG_INFO(format, ...) \
    log_message("INFO", ANSI_ESCAPE_CODE_GREEN, format, ##__VA_ARGS__)

#define LOG_DEBUG(format, ...) \
    log_message("DEBUG", ANSI_ESCAPE_CODE_BLUE, format, ##__VA_ARGS__)

#define LOG_TRACE(format, ...) \
    log_message("TRACE", ANSI_ESCAPE_CODE_BLUE, format, ##__VA_ARGS__)
