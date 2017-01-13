#pragma once


namespace Hilltop {
namespace Console {

enum ConsoleColor : unsigned char {
    BLACK = 0,
    DARK_BLUE = 1,
    DARK_GREEN = 2,
    BROWN = 3,
    PURPLE = 4,
    ORANGE = 5,
    INDIGO = 6,
    DARK_GRAY = 7,
    GRAY = 8,
    BLUE = 9,
    GREEN = 10,
    RED = 11,
    PINK = 12,
    YELLOW = 13,
    PEACH = 14,
    WHITE = 15,
};

enum ConsoleColorType : unsigned char {
    FOREGROUND_COLOR = 0xf,
    BACKGROUND_COLOR = 0xf0,
    FOREGROUND_SHIFT = 0,
    BACKGROUND_SHIFT = 4,
    MAX_COLOR_VALUE = FOREGROUND_COLOR | BACKGROUND_COLOR
};

ConsoleColor make_color(ConsoleColor background, ConsoleColor foreground);
ConsoleColor make_bg_color(ConsoleColor background);
ConsoleColor make_fg_color(ConsoleColor foreground);
ConsoleColor calc_masked_color(ConsoleColor old, ConsoleColor color, ConsoleColorType mask);

bool is_bright_color(ConsoleColor color);
bool is_dark_color(ConsoleColor color);

}
}
