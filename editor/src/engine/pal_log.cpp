#include "pal_log.h"
#include <cstdio>
#include <cstdlib>

namespace engine {
PalLog PalLog::_log;

const char* PalLog::getLevelStr(LogLevel level)
{
    switch (level) {
    case LogLevel::debug: {
        return "[debug] ";
    } break;
    case LogLevel::info: {
        return "[ info] ";
    } break;
    case LogLevel::warn: {
        return "[ warn] ";
    } break;
    case LogLevel::error: {
        return "[error] ";
    } break;
    default:
        return "";
    }
}

void PalLog::clearLog()
{
    for (int i = 0; i < Items.Size; i++)
        free(Items[i]);
    Items.clear();
}

void PalLog::addLogAp(LogLevel level, const char* fmt, va_list args)
{
    char buf[2048];
    char* start = buf;
    const char* prefix = getLevelStr(level);
    int prefix_len = strlen(prefix);
    if (prefix && prefix_len) {
        memcpy(buf, prefix, strlen(prefix));
        start += prefix_len;
    }
    vsnprintf(start, IM_ARRAYSIZE(buf) - prefix_len, fmt, args);
    buf[IM_ARRAYSIZE(buf) - 1] = 0;
    Items.push_back(strdup(buf));
}

void PalLog::addLog(LogLevel level, const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    addLogAp(level, fmt, args);
    va_end(args);
}

}