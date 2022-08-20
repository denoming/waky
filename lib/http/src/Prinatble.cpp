#include "http/Prinatble.hpp"

#include <cstdio>
#include <cstdarg>
#include <cstring>

int
Printable::printf(const char* format, ...)
{
    char buffer[64];
    char* ptr = buffer;
    va_list arg;
    va_list copy;
    va_start(arg, format);
    va_copy(copy, arg);
    int len = vsnprintf(ptr, sizeof(buffer), format, copy);
    va_end(copy);
    if (len < 0) {
        va_end(arg);
        return -1;
    };
    if (len >= sizeof(buffer)) {
        ptr = static_cast<char*>(malloc(len + 1));
        if (ptr == NULL) {
            va_end(arg);
            return -1;
        }
        len = vsnprintf(ptr, len + 1, format, arg);
    }
    va_end(arg);
    len = write(ptr, len);
    if (ptr != buffer) {
        free(ptr);
    }
    return len;
}

int
Printable::print(const char* data)
{
    return write(data, strlen(data));
}

int
Printable::print(const char* data, size_t size)
{
    return write(data, size);
}