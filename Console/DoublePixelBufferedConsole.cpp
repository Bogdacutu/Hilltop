#include "DoublePixelBufferedConsole.h"


namespace Hilltop {
namespace Console {

const uint8_t Hilltop::Console::DoublePixelBufferedConsole::BIT_MASKS[2] = { 0xf0, 0xf };
const int Hilltop::Console::DoublePixelBufferedConsole::BIT_SHIFTS[2] = { 4, 0 };

Hilltop::Console::DoublePixelBufferedConsole::DoublePixelBufferedConsole(unsigned short width,
    unsigned short height) : width((width + 1) / 2 * 2), height((height + 1) / 2 * 2) {
    buffer = std::vector<uint8_t>(this->width * (this->height) / 2);
}

ConsoleColor Hilltop::Console::DoublePixelBufferedConsole::get(unsigned short x, unsigned short y) const {
    if (x >= height || y >= width)
        return ConsoleColor();

    const unsigned int idx = x * width + y;
    return (ConsoleColor)((buffer[idx / 2] & BIT_MASKS[idx & 1]) >> BIT_SHIFTS[idx & 1]);
}

void Hilltop::Console::DoublePixelBufferedConsole::set(unsigned short x, unsigned short y,
    ConsoleColor color) {
    if (x >= height || y >= width)
        return;

    const unsigned int idx = x * width + y;
    buffer[idx / 2] = (buffer[idx / 2] & BIT_MASKS[!(idx & 1)])
        | ((color << BIT_SHIFTS[idx & 1]) & BIT_MASKS[idx & 1]);
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

}
}
