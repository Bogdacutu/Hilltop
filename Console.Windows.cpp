#include "Console.h"
#include <Windows.h>
#include "Console.Windows.h"

using namespace Hilltop::Console;

static void setConsoleFontSize(HANDLE buffer, unsigned short size) {
    CONSOLE_FONT_INFOEX info = { sizeof(info) };
    GetCurrentConsoleFontEx(buffer, FALSE, &info);
    info.dwFontSize.X = size;
    info.dwFontSize.Y = size * 2;
    SetCurrentConsoleFontEx(buffer, FALSE, &info);
}

static void setConsoleSize(HANDLE buffer, unsigned short width, unsigned short height) {
    CONSOLE_SCREEN_BUFFER_INFO info;
    GetConsoleScreenBufferInfo(buffer, &info);
    SetConsoleScreenBufferSize(buffer, { max(info.dwSize.X, width), max(info.dwSize.Y, height) });
    SMALL_RECT finalArea = { 0, 0, (SHORT)width - 1, (SHORT)height - 1 };
    SetConsoleWindowInfo(buffer, TRUE, &finalArea);
    SetConsoleScreenBufferSize(buffer, { (SHORT)width, (SHORT)height });
}

static void getConsoleSize(HANDLE buffer, unsigned short *width, unsigned short *height) {
    CONSOLE_SCREEN_BUFFER_INFO info;
    GetConsoleScreenBufferInfo(buffer, &info);
    *width = info.srWindow.Right - info.srWindow.Left + 1;
    *height = info.srWindow.Bottom - info.srWindow.Top + 1;
}

static bool resizeWithAutoFont(HANDLE buffer, unsigned short width, unsigned short height, unsigned short minFont) {
    unsigned short w = 0, h = 0;
    for (unsigned short i = 72; i >= minFont; i--) {
        setConsoleFontSize(buffer, i);
        setConsoleSize(buffer, width, height);
        getConsoleSize(buffer, &w, &h);
        if (w == width && h == height)
            return true;
    }
    return false;
}

static void setGameBufferProps(HANDLE buffer) {
    CONSOLE_SCREEN_BUFFER_INFOEX info = { sizeof(info) };
    GetConsoleScreenBufferInfoEx(buffer, &info);
    info.srWindow.Bottom++;

    // http://www.romanzolotarev.com/pico-8-color-palette/
    info.ColorTable[BLACK] = RGB(0, 0, 0);
    info.ColorTable[DARK_BLUE] = RGB(29, 43, 83);
    info.ColorTable[PURPLE] = RGB(126, 37, 83);
    info.ColorTable[DARK_GREEN] = RGB(0, 135, 81);
    info.ColorTable[BROWN] = RGB(171, 82, 54);
    info.ColorTable[DARK_GRAY] = RGB(95, 87, 79);
    info.ColorTable[GRAY] = RGB(194, 195, 199);
    info.ColorTable[WHITE] = RGB(255, 241, 232);
    info.ColorTable[RED] = RGB(255, 0, 71);
    info.ColorTable[ORANGE] = RGB(255, 163, 0);
    info.ColorTable[YELLOW] = RGB(255, 236, 39);
    info.ColorTable[GREEN] = RGB(0, 228, 54);
    info.ColorTable[BLUE] = RGB(41, 173, 255);
    info.ColorTable[INDIGO] = RGB(131, 118, 156);
    info.ColorTable[PINK] = RGB(255, 119, 168);
    info.ColorTable[PEACH] = RGB(255, 204, 170);

    SetConsoleScreenBufferInfoEx(buffer, &info);
}

Hilltop::Console::WindowsConsole::WindowsConsole(HANDLE handle, unsigned short width, unsigned short height,
    unsigned short minFont) : BufferedConsole(width, height), handle(handle) {
    resizeWithAutoFont(handle, width, height, minFont);
    setGameBufferProps(handle);
}

BufferedConsole::pixel_t Hilltop::Console::WindowsConsole::get(unsigned short x, unsigned short y) const {
    if (x >= height || y >= width)
        return pixel_t();

    const unsigned int idx = x * width + y;
    pixel_t ret;
    ret.ch = buffer[idx].Char.UnicodeChar;
    ret.color = (ConsoleColor)buffer[idx].Attributes;
    return ret;
}

void Hilltop::Console::WindowsConsole::set(unsigned short x, unsigned short y, wchar_t ch, ConsoleColor color,
    ConsoleColorType colorMask) {
    if (x >= height || y >= width)
        return;

    const unsigned int idx = x * width + y;
    buffer[idx].Char.UnicodeChar = ch;
    buffer[idx].Attributes = calc_masked_color((ConsoleColor)buffer[idx].Attributes, color, colorMask);
}

void Hilltop::Console::WindowsConsole::commit() const {
    SMALL_RECT area = { 0, 0, (SHORT)width, (SHORT)height };
    if (!WriteConsoleOutput(GetStdHandle(STD_OUTPUT_HANDLE), &buffer[0], { (SHORT)width, (SHORT)height }, { 0, 0 }, &area))
        abort();
}