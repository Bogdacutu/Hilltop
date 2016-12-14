#include "Console.h"
#include <Windows.h>

using namespace Hilltop::Console;

void ConsoleBuffer::commit() {
    std::vector<CHAR_INFO> data(width * height);

    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            const unsigned int idx = i * width + j;
            data[idx].Char.UnicodeChar = buffer[idx].ch;
            data[idx].Attributes = buffer[idx].color;
        }
    }

    SMALL_RECT area = { 0, 0, (SHORT)width, (SHORT)height };
    if (!WriteConsoleOutput(GetStdHandle(STD_OUTPUT_HANDLE), &data[0], { (SHORT)width, (SHORT)height }, { 0, 0 }, &area))
        abort();
}
