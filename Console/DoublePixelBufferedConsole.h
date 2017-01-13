#pragma once

#include "Console.h"
#include <vector>


namespace Hilltop {
namespace Console {

class DoublePixelBufferedConsole {
private:
    std::vector<uint8_t> buffer;

    static const uint8_t BIT_MASKS[2];
    static const int BIT_SHIFTS[2];

public:
    const unsigned short width, height;

    DoublePixelBufferedConsole(unsigned short width, unsigned short height);

    ConsoleColor get(unsigned short x, unsigned short y) const;
    void set(unsigned short x, unsigned short y, ConsoleColor color);
    void clear(ConsoleColor color);

    void commit(Console &buffer) const;
};

}
}
