#include "bitmap_image.hpp"
#include <iostream>
#include <Windows.h>
#include "Console.h"
#include <sstream>
#include <deque>

using namespace Hilltop::Console;

int width = 178;
int height = 130;

BufferedConsole console(width, (height + 1) / 2);

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
    for (unsigned short i = 72; i >= minFont; i--) {
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

    if (!resizeWithAutoFont(GetStdHandle(STD_OUTPUT_HANDLE), width, (height + 1) / 2, 2))
        abort();

    DWORD lastTicks = GetTickCount();

    RollingAverage frametime(25);

    DoublePixelConsoleBuffer buffer(width, height);

    /*while (true) {
        for (int i = 0; i < width; i++) {
            for (int j = 0; j < height; j++) {
                const int f = 1;
                if (!((i < width / f && j < height / f) || (i >= (width - width / f) && j >= (height - height / f))))
                    continue;
                buffer.set(j, i, make_fg_color((ConsoleColor)(color + i + j / 3)));
            }
        }

        buffer.commit(console);

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
    }*/

    bitmap_image image("map.bmp");

    for (unsigned short i = 0; i < image.height(); i++) {
        for (unsigned short j = 0; j < image.width(); j++) {
            rgb_t p;
            image.get_pixel(j, i, p);
            if (p.green >= 128) {
                buffer.set(i, j, GREEN);
            }
        }
    }

    buffer.commit(console);
    console.commit();

    int it = 0;
    int pos = 0;
    int rep = 0;
    int off = 0;

    while (true) {
        for (int i = 0; i < 5; i++) {
            int x = sin(it++ / 10.) * 10 + 14;
            if (x == 14) rep++;
            if (rep == 4) {
                rep = 0;
                off++;
            }
            buffer.set(x + off, pos, off % 2 == 0 ? RED : BLUE);
            //buffer.set(x + off + 50, pos, RED);
            //buffer.set(x + off + 100, pos, RED);
            pos = (pos + 1) % width;
        }

        buffer.commit(console);
        console.commit();

        //Sleep(1);
    }
}
