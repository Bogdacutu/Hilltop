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

void Hilltop::Console::ConsoleBuffer::set(unsigned short x, unsigned short y, char ch, ConsoleColor color, ConsoleColorType colorMask) {
    const unsigned int idx = x * width + y;
    buffer[idx].ch = ch;
    buffer[idx].color = (buffer[idx].color & ~colorMask) | (color & colorMask);
}

void Hilltop::Console::ConsoleBuffer::clear(ConsoleColor color) {
    for (int i = 0; i < height; i++)
        for (int j = 0; j < width; j++)
            set(i, j, ' ', color);
}
