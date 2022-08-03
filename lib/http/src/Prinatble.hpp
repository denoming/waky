#pragma once

#include <cstddef>

class Printable {
public:
    int
    printf(const char* format, ...);

    int
    print(const char* data);

    int
    print(const char* data, size_t size);

protected:
    virtual int
    write(const char* data, size_t size)
        = 0;
};