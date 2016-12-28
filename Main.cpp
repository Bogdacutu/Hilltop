#include <iostream>
#include <Windows.h>
#include "Console.h"
#include "Console.Windows.h"
#include <sstream>
#include <deque>

using namespace Hilltop::Console;

unsigned short width = 178;
unsigned short height = 130;

WindowsConsole rawConsole(GetStdHandle(STD_OUTPUT_HANDLE), width + 4, (height + 1) / 2 + 2);
BufferedConsoleRegion console(std::shared_ptr<WindowsConsole>(&rawConsole, [](WindowsConsole *) {}), width, (height + 1) / 2, 1, 2);

void printString(std::string str, unsigned short x, unsigned short y) {
    for (int i = 0; i < str.length(); i++)
        console.set(x, y + i, str[i], make_color(BLACK, WHITE));
}

template<typename T>
class RollingAverage {
    std::deque<T> Q;
    int count;

public:
    RollingAverage(int count) : count(count) {}

    void add(T value) {
        while (Q.size() >= count)
            Q.pop_front();
        while (Q.size() < count)
            Q.push_back(value);
    }

    T get() {
        T s = 0;
        for (T &x : Q)
            s += x;
        return s / count;
    }
};

int main() {
    AttachConsole(-1);

    ConsoleColor color = RED;

    ULONGLONG lastTicks = GetTickCount64();
    ULONGLONG lastSec = lastTicks / 1000 - 1;

    RollingAverage<ULONGLONG> frametime(25);

    DoublePixelBufferedConsole buffer(width, height);

    while (true) {
        if (lastSec != lastTicks / 1000) {
            for (int i = 0; i < width; i++) {
                for (int j = 0; j < height; j++) {
                    const int f = 1;
                    if (!((i < width / f && j < height / f) || (i >= (width - width / f) && j >= (height - height / f))))
                        continue;
                    buffer.set(j, i, make_fg_color((ConsoleColor)(color + i + j / 2 + j / 1 % 2 * 8)));
                }
            }

            color = make_fg_color((ConsoleColor)(color + 1));
            lastSec = lastTicks / 1000;
        }

        int progress = lastTicks % 2000 / 100 + 6;
        const ConsoleColor PROGRESS_COLORS[3] = { GRAY, DARK_GRAY, BLACK };
        for (int i = 0; i < 3; i++)
            buffer.set(i + 2, progress, PROGRESS_COLORS[i]);

        buffer.commit(console);

        ULONGLONG ticks = timeGetTime();
        ULONGLONG time = ticks - lastTicks;
        frametime.add(time);

        std::ostringstream msg1;
        msg1 << "Frame time: " << frametime.get() << " ms";
        std::ostringstream msg2;
        msg2 << "FPS: " << 1000 / max(1, frametime.get());
        printString(msg1.str(), 3, 6);
        printString(msg2.str(), 4, 6);

        lastTicks = ticks;

        rawConsole.commit();

        Sleep(1);
    }
}
