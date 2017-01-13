#include "Console/Windows/WindowsConsole.h"
#include <algorithm>

using namespace Hilltop::Console;


static unsigned short getConsoleFontSize(HANDLE buffer) {
    CONSOLE_FONT_INFOEX info = { sizeof(info) };
    GetCurrentConsoleFontEx(buffer, FALSE, &info);
    return info.dwFontSize.X;
}

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
    SetConsoleScreenBufferSize(buffer, { std::max(info.dwSize.X, (SHORT)width),
        std::max(info.dwSize.Y, (SHORT)height) });
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

static void centerWindow(HWND window) {
    RECT pos;
    GetWindowRect(window, &pos);
    int width = pos.right - pos.left;
    int height = pos.bottom - pos.top;
    HMONITOR monitor = MonitorFromWindow(window, MONITOR_DEFAULTTOPRIMARY);
    MONITORINFO info = { sizeof(MONITORINFO) };
    GetMonitorInfo(monitor, &info);
    int monWidth = info.rcMonitor.right - info.rcMonitor.left;
    SetWindowPos(window, HWND_TOP, (monWidth - width) / 2, 0, width, height, 0);
}

static void centerConsoleWindow() {
    centerWindow(GetConsoleWindow());
}

static unsigned short resizeWithAutoFont(HANDLE buffer, unsigned short width, unsigned short height,
    unsigned short minFont, unsigned short maxFont) {
    unsigned short w = 0, h = 0;
    for (unsigned short i = maxFont; i >= minFont; i--) {
        setConsoleFontSize(buffer, i);
        setConsoleSize(buffer, width, height);
        getConsoleSize(buffer, &w, &h);
        if (w == width && h == height)
            return i;
    }
    return minFont - 1;
}


COLORREF colors[16];


static void setGameBufferProps(HANDLE buffer) {
    CONSOLE_SCREEN_BUFFER_INFOEX info = { sizeof(info) };
    GetConsoleScreenBufferInfoEx(buffer, &info);
    memcpy(info.ColorTable, colors, sizeof(colors));

    SetConsoleScreenBufferInfoEx(buffer, &info);

    CONSOLE_CURSOR_INFO cursorInfo;
    cursorInfo.dwSize = 1;
    cursorInfo.bVisible = FALSE;
    SetConsoleCursorInfo(buffer, &cursorInfo);
}


void Hilltop::Console::initWindowsColors() {
    // http://www.romanzolotarev.com/pico-8-color-palette/
    colors[BLACK] = RGB(0, 0, 0);
    colors[DARK_BLUE] = RGB(29, 43, 83);
    colors[PURPLE] = RGB(126, 37, 83);
    colors[DARK_GREEN] = RGB(0, 135, 81);
    colors[BROWN] = RGB(171, 82, 54);
    colors[DARK_GRAY] = RGB(95, 87, 79);
    colors[GRAY] = RGB(194, 195, 199);
    colors[WHITE] = RGB(255, 255, 255);
    colors[RED] = RGB(225, 0, 0);
    colors[ORANGE] = RGB(255, 163, 0);
    colors[YELLOW] = RGB(255, 236, 39);
    colors[GREEN] = RGB(0, 228, 54);
    colors[BLUE] = RGB(41, 173, 255);
    colors[INDIGO] = RGB(131, 118, 156);
    colors[PINK] = RGB(255, 119, 168);
    colors[PEACH] = RGB(255, 204, 170);
}

COLORREF Hilltop::Console::mapWindowsColor(ConsoleColor color) {
    return colors[color];
}


Hilltop::Console::WindowsConsole::WindowsConsole(HANDLE handle, unsigned short width,
    unsigned short height, unsigned short minFont, unsigned short maxFont)
    : BufferedNativeConsole(width, height), handle(handle), minFont(minFont), maxFont(maxFont) {
    configure();
}

void Hilltop::Console::WindowsConsole::configure() {
    if (chosenSize > 0) {
        if (getConsoleFontSize(handle) != chosenSize)
            setConsoleFontSize(handle, chosenSize);
        setConsoleSize(handle, width, height);
    } else {
        setGameBufferProps(handle);
        chosenSize = resizeWithAutoFont(handle, width, height, minFont, maxFont);
    }
    centerConsoleWindow();
}

std::shared_ptr<WindowsConsole> Hilltop::Console::WindowsConsole::create(HANDLE handle, unsigned short width,
    unsigned short height, unsigned short minFont, unsigned short maxFont) {
    return std::shared_ptr<WindowsConsole>(new WindowsConsole(handle, width, height, minFont, maxFont));
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
