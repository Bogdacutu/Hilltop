#include "BufferedConsole.h"

using namespace Hilltop::Console;


Hilltop::Console::BufferedConsole::BufferedConsole(unsigned short width, unsigned short height)
    : Console(width, height) {}

void Hilltop::Console::BufferedConsole::set(unsigned short x, unsigned short y, wchar_t ch,
    ConsoleColor color) {
    set(x, y, ch, color, (ConsoleColorType)(BACKGROUND_COLOR | FOREGROUND_COLOR));
}
