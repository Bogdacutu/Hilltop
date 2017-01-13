#include "ConsoleColor.h"

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

bool Hilltop::Console::is_bright_color(ConsoleColor color) {
    return color >= GRAY;
}

bool Hilltop::Console::is_dark_color(ConsoleColor color) {
    return color <= DARK_GRAY;
}
