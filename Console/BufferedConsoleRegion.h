#pragma once

#include "BufferedConsole.h"


namespace Hilltop {
namespace Console {

class BufferedConsoleRegion : public BufferedConsole {
protected:
    BufferedConsoleRegion(BufferedConsole &console, unsigned short width, unsigned short height,
        unsigned short x, unsigned short y);

public:
    const std::shared_ptr<BufferedConsole> console;
    const unsigned short x, y;

    bool enforceBounds = true;

    static std::shared_ptr<BufferedConsoleRegion> create(BufferedConsole &console,
        unsigned short width, unsigned short height, unsigned short x, unsigned short y);

    virtual pixel_t get(unsigned short x, unsigned short y) const override;
    virtual void set(unsigned short x, unsigned short y, wchar_t ch, ConsoleColor color) override;
    virtual void set(unsigned short x, unsigned short y, wchar_t ch, ConsoleColor color,
        ConsoleColorType colorMask) override;
};

}
}
