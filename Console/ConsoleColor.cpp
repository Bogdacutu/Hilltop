#include "ConsoleColor.h"


namespace Hilltop {
namespace Console {

ConsoleColor make_color(ConsoleColor background, ConsoleColor foreground) {
    return (ConsoleColor)(((background << BACKGROUND_SHIFT) & BACKGROUND_COLOR)
        | ((foreground << FOREGROUND_SHIFT) & FOREGROUND_COLOR));
}

ConsoleColor make_bg_color(ConsoleColor background) {
    return make_color(background, (ConsoleColor)0);
}

ConsoleColor make_fg_color(ConsoleColor foreground) {
    return make_color((ConsoleColor)0, foreground);
}

ConsoleColor calc_masked_color(ConsoleColor old, ConsoleColor color, ConsoleColorType mask) {
    return (ConsoleColor)((old & ~mask) | (color & mask));
}

bool is_bright_color(ConsoleColor color) {
    return color >= GRAY;
}

bool is_dark_color(ConsoleColor color) {
    return color <= DARK_GRAY;
}

}
}
