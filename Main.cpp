﻿#include <iostream>
#include <Windows.h>
#include "Console.h"
#include <sstream>
#include <deque>

using namespace Hilltop::Console;

const int width = 300;
const int height = 130 / 2;

BufferedConsole console(width, height);

void setConsoleFontSize(HANDLE buffer, unsigned short size) {
    CONSOLE_FONT_INFOEX info = { sizeof(info) };
    GetCurrentConsoleFontEx(buffer, FALSE, &info);
    info.dwFontSize.X = size;
    info.dwFontSize.Y = size * 2;
    SetCurrentConsoleFontEx(buffer, FALSE, &info);
}

void setConsoleSize(HANDLE buffer, unsigned short width, unsigned short height) {
    CONSOLE_SCREEN_BUFFER_INFO info;
    GetConsoleScreenBufferInfo(buffer, &info);
    SetConsoleScreenBufferSize(buffer, { max(info.dwSize.X, width), max(info.dwSize.Y, height) });
    SMALL_RECT finalArea = { 0, 0, (SHORT)width - 1, (SHORT)height - 1 };
    SetConsoleWindowInfo(buffer, TRUE, &finalArea);
    SetConsoleScreenBufferSize(buffer, { (SHORT)width, (SHORT)height });
}

void getConsoleSize(HANDLE buffer, unsigned short *width, unsigned short *height) {
    CONSOLE_SCREEN_BUFFER_INFO info;
    GetConsoleScreenBufferInfo(buffer, &info);
    *width = info.srWindow.Right - info.srWindow.Left + 1;
    *height = info.srWindow.Bottom - info.srWindow.Top + 1;
}

void printString(std::string str, unsigned short x, unsigned short y) {
    for (int i = 0; i < str.length(); i++)
        console.set(x, y + i, str[i], make_color(BLACK, WHITE));
}

bool resizeWithAutoFont(HANDLE buffer, unsigned short width, unsigned short height, unsigned short minFont) {
    unsigned short w = 0, h = 0;
    for (unsigned short i = 10; i >= minFont; i--) {
        setConsoleFontSize(buffer, i);
        setConsoleSize(buffer, width, height);
        getConsoleSize(buffer, &w, &h);
        if (w == width && h == height)
            return true;
    }
    return false;
}

class RollingAverage {
    std::deque<int> Q;
    int count;

public:
    RollingAverage(int count) : count(count) {}

    void add(int value) {
        while (Q.size() >= count)
            Q.pop_front();
        while (Q.size() < count)
            Q.push_back(value);
    }

    int get() {
        int s = 0;
        for (int &x : Q)
            s += x;
        return s / count;
    }
};

int main() {
    AttachConsole(-1);

    ConsoleColor color = RED;

    if (!resizeWithAutoFont(GetStdHandle(STD_OUTPUT_HANDLE), width, height, 4))
        abort();

    DWORD lastTicks = GetTickCount();

    RollingAverage frametime(25);

    while (true) {
        for (int i = 0; i < width; i++) {
            for (int j = 0; j < height; j++) {
                const int f = 1;
                if (!((i < width / f && j < height / f) || (i >= (width - width / f) && j >= (height - height / f))))
                    continue;
                ConsoleColor c = make_fg_color((ConsoleColor)(color + i + j / 5));
                console.set(j, i, ' ', make_bg_color(c), BACKGROUND_COLOR);
                char ch = '0' + c % 16;
                if (ch > '9') ch = ch - '9' - 1 + 'a';
                if (j == 0)
                    console.set(j, i, ch, make_fg_color((ConsoleColor)~c), FOREGROUND_COLOR);
                else if ((color + i + j / 5) % 4 == 2)
                    console.set(j, i, ':', make_fg_color((ConsoleColor)~c), FOREGROUND_COLOR);
                else if ((color + i + j / 5) % 4 == 3)
                    console.set(j, i, ')', make_fg_color((ConsoleColor)~c), FOREGROUND_COLOR);
            }
        }

        DWORD ticks = timeGetTime();
        DWORD time = ticks - lastTicks;
        frametime.add(time);

        std::ostringstream msg1;
        msg1 << "Frame time: " << frametime.get() << " ms";
        std::ostringstream msg2;
        msg2 << "FPS: " << 1000 / max(1, frametime.get());
        printString(msg1.str(), 3, 6);
        printString(msg2.str(), 4, 6);

        lastTicks = ticks;

        console.commit();

        color = make_fg_color((ConsoleColor)(color + 1));

        Sleep(1);
    }
}
