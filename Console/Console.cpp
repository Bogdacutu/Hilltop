#include "Console.h"


namespace Hilltop {
namespace Console {

Console::Console(unsigned short width, unsigned short height)
    : width(width), height(height) {}

void Console::clear(ConsoleColor color) {
    color = make_bg_color(color);
    for (int i = 0; i < height; i++)
        for (int j = 0; j < width; j++)
            set(i, j, ' ', color);
}

}
}
