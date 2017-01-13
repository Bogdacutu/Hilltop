#include "BufferedConsoleRegion.h"


namespace Hilltop {
namespace Console {

BufferedConsoleRegion::BufferedConsoleRegion(BufferedConsole &console,
    unsigned short width, unsigned short height, unsigned short x, unsigned short y)
    : BufferedConsole(width, height), x(x), y(y)
    , console(std::static_pointer_cast<BufferedConsole>(console.shared_from_this())) {}

std::shared_ptr<BufferedConsoleRegion> BufferedConsoleRegion::create(
    BufferedConsole& console, unsigned short width, unsigned short height, unsigned short x,
    unsigned short y) {
    return std::shared_ptr<BufferedConsoleRegion>(new BufferedConsoleRegion(console, width, height, x, y));
}

BufferedConsole::pixel_t BufferedConsoleRegion::get(unsigned short x, unsigned short y) const {
    if (enforceBounds)
        if (x >= height || y >= width)
            return pixel_t();

    return console->get(this->x + x, this->y + y);
}

void BufferedConsoleRegion::set(unsigned short x, unsigned short y, wchar_t ch,
    ConsoleColor color) {
    BufferedConsole::set(x, y, ch, color);
}

void BufferedConsoleRegion::set(unsigned short x, unsigned short y, wchar_t ch,
    ConsoleColor color, ConsoleColorType colorMask) {
    if (enforceBounds)
        if (x >= height || y >= width)
            return;

    console->set(this->x + x, this->y + y, ch, color, colorMask);
}

}
}
