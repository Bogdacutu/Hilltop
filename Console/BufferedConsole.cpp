#include "BufferedConsole.h"


namespace Hilltop {
namespace Console {

BufferedConsole::BufferedConsole(unsigned short width, unsigned short height)
    : Console(width, height) {}

void BufferedConsole::set(unsigned short x, unsigned short y, wchar_t ch,
    ConsoleColor color) {
    set(x, y, ch, color, (ConsoleColorType)(BACKGROUND_COLOR | FOREGROUND_COLOR));
}

}
}
