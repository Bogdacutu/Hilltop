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

Hilltop::Console::ConsoleBuffer::ConsoleBuffer(unsigned short width, unsigned short height)
    : width(width), height(height), buffer(width * height) {}

void Hilltop::Console::ConsoleBuffer::set(unsigned short x, unsigned short y, wchar_t ch, ConsoleColor color,
    ConsoleColorType colorMask) {
    const unsigned int idx = x * width + y;
    buffer[idx].ch = ch;
    buffer[idx].color = (buffer[idx].color & ~colorMask) | (color & colorMask);
}

void Hilltop::Console::ConsoleBuffer::clear(ConsoleColor color) {
    buffer.assign(buffer.size(), { L' ', (uint8_t)color });
}

Hilltop::Console::DoublePixelConsoleBuffer::DoublePixelConsoleBuffer(unsigned short width, unsigned short height)
    : width((width + 1) / 2 * 2), height((height + 1) / 2 * 2) {
    buffer = std::vector<uint8_t>(this->width * (this->height) / 2);
}

ConsoleColor Hilltop::Console::DoublePixelConsoleBuffer::get(unsigned short x, unsigned short y) {
    const unsigned int idx = x * width + y;
    return (ConsoleColor)((buffer[idx / 2] & BIT_MASKS[idx & 1]) >> BIT_SHIFTS[idx & 1]);
}

void Hilltop::Console::DoublePixelConsoleBuffer::set(unsigned short x, unsigned short y, ConsoleColor color) {
    const unsigned int idx = x * width + y;
    buffer[idx / 2] = (buffer[idx / 2] & BIT_MASKS[!(idx & 1)]) | ((color << BIT_SHIFTS[idx & 1]) & BIT_MASKS[idx & 1]);
}

void Hilltop::Console::DoublePixelConsoleBuffer::clear(ConsoleColor color) {
    uint8_t value = 0;
    for (int i = 0; i < sizeof(BIT_MASKS) / sizeof(*BIT_MASKS); i++)
        value |= (color << BIT_SHIFTS[i]) & BIT_MASKS[i];
    buffer.assign(buffer.size(), value);
}

void Hilltop::Console::DoublePixelConsoleBuffer::commit(ConsoleBuffer &buffer, unsigned int x, unsigned int y) {
    for (int i = 0; i < height / 2; i++)
        for (int j = 0; j < width; j++)
            buffer.set(x + i, y + j, L'▄', (ConsoleColor)((get(i * 2, j) << BACKGROUND_SHIFT) | get(i * 2 + 1, j)));
}
