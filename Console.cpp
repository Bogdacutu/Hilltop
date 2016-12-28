#include "Console.h"

using namespace Hilltop::Console;

ConsoleColor Hilltop::Console::make_color(ConsoleColor background, ConsoleColor foreground) {
    return (ConsoleColor)(((background << BACKGROUND_SHIFT) & BACKGROUND_COLOR)
        | ((foreground << FOREGROUND_SHIFT) & FOREGROUND_COLOR));
}

ConsoleColor Hilltop::Console::make_bg_color(ConsoleColor background) {
    return make_color(background, (ConsoleColor)0);
}

ConsoleColor Hilltop::Console::make_fg_color(ConsoleColor foreground) {
    return make_color((ConsoleColor)0, foreground);
}

ConsoleColor Hilltop::Console::calc_masked_color(ConsoleColor old, ConsoleColor color, ConsoleColorType mask) {
    return (ConsoleColor)((old & ~mask) | (color & mask));
}

Hilltop::Console::Console::Console(unsigned short width, unsigned short height)
    : width(width), height(height) {}

void Hilltop::Console::Console::clear(ConsoleColor color) {
    for (int i = 0; i < width; i++)
        for (int j = 0; j < height; j++)
            set(i, j, ' ', color);
}

Hilltop::Console::BufferedConsole::BufferedConsole(unsigned short width, unsigned short height)
    : Console(width, height) {}

Hilltop::Console::BufferedConsoleRegion::BufferedConsoleRegion(std::shared_ptr<BufferedConsole> console,
    unsigned short width, unsigned short height, unsigned short x, unsigned short y)
    : BufferedConsole(width, height), x(x), y(y), console(console) {}

BufferedConsole::pixel_t Hilltop::Console::BufferedConsoleRegion::get(unsigned short x, unsigned short y) const {
    if (enforceBounds)
        if (x >= height || y >= width)
            return pixel_t();

    return console->get(this->x + x, this->y + y);
}

void Hilltop::Console::BufferedConsoleRegion::set(unsigned short x, unsigned short y, wchar_t ch,
    ConsoleColor color, ConsoleColorType colorMask) {
    if (enforceBounds)
        if (x >= height || y >= width)
            return;

    console->set(this->x + x, this->y + y, ch, color, colorMask);
}

bool Hilltop::Console::BufferedConsoleRegion::enforcingBounds() const {
    return enforceBounds;
}

void Hilltop::Console::BufferedConsoleRegion::setEnforceBounds(bool value) {
    enforceBounds = value;
}

const uint8_t Hilltop::Console::DoublePixelBufferedConsole::BIT_MASKS[2] = { 0xf0, 0xf };
const int Hilltop::Console::DoublePixelBufferedConsole::BIT_SHIFTS[2] = { 4, 0 };

Hilltop::Console::DoublePixelBufferedConsole::DoublePixelBufferedConsole(unsigned short width, unsigned short height)
    : width((width + 1) / 2 * 2), height((height + 1) / 2 * 2) {
    buffer = std::vector<uint8_t>(this->width * (this->height) / 2);
}

ConsoleColor Hilltop::Console::DoublePixelBufferedConsole::get(unsigned short x, unsigned short y) const {
    const unsigned int idx = x * width + y;
    return (ConsoleColor)((buffer[idx / 2] & BIT_MASKS[idx & 1]) >> BIT_SHIFTS[idx & 1]);
}

void Hilltop::Console::DoublePixelBufferedConsole::set(unsigned short x, unsigned short y, ConsoleColor color) {
    const unsigned int idx = x * width + y;
    buffer[idx / 2] = (buffer[idx / 2] & BIT_MASKS[!(idx & 1)]) | ((color << BIT_SHIFTS[idx & 1]) & BIT_MASKS[idx & 1]);
}

void Hilltop::Console::DoublePixelBufferedConsole::clear(ConsoleColor color) {
    uint8_t value = 0;
    for (int i = 0; i < sizeof(BIT_MASKS) / sizeof(*BIT_MASKS); i++)
        value |= (color << BIT_SHIFTS[i]) & BIT_MASKS[i];
    buffer.assign(buffer.size(), value);
}

void Hilltop::Console::DoublePixelBufferedConsole::commit(Console &buffer) const {
    for (unsigned short i = 0; i < height / 2; i++) {
        for (unsigned short j = 0; j < width; j++) {
            const ConsoleColor x = get(i * 2, j);
            const ConsoleColor y = get(i * 2 + 1, j);
            const wchar_t c = x == y ? ' ' : L'▄';
            buffer.set(i, j, c, make_color(x, y));
        }
    }
}
