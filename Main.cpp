﻿#include "Console.h"
#include "Console.Windows.h"
#include "Game.h"
#include <deque>
#include <iostream>
#include <memory>
#include <sstream>
#include <Windows.h>

#define GAME_TICKS_PER_SEC 20
#define GAME_TICK_MS (1000 / GAME_TICKS_PER_SEC)

using namespace Hilltop::Console;
using namespace Hilltop::Game;

enum engine_state_t {
    STATE_MENU,
    STATE_INGAME,
};

engine_state_t state = STATE_INGAME;
bool consoleEnabled = false;

const unsigned short MENU_WIDTH = 80;
const unsigned short MENU_HEIGHT = 25;

const unsigned short GAME_WIDTH = 200;
const unsigned short GAME_HEIGHT = 50;

std::shared_ptr<BufferedConsole> console;

int main() {
    srand(GetTickCount());
    AttachConsole(-1);

    console = WindowsConsole::create(GetStdHandle(STD_OUTPUT_HANDLE), GAME_WIDTH, GAME_HEIGHT);

    TankMatch match(GAME_WIDTH - 4, GAME_HEIGHT * 2 - 4);
    match.buildMap([](float x) { return (std::sinf(x * 2 - 1.4f) + 1) / 2 + 0.1f; });

    for (int i = 0; i < GAME_WIDTH - 4; i += 10) {
        std::shared_ptr<Tank> tank = Tank::create(BLUE);
        tank->position = { 0, (float)i };
        tank->angle = (i * 2) % 360;
        tank->initWheels(&match);
        match.addEntity(*tank);
    }

    std::shared_ptr<BufferedConsoleRegion> mainRegion = BufferedConsoleRegion::create(*console,
        GAME_WIDTH - 2, GAME_HEIGHT - 1, 1, 2);

    ULONGLONG lastTime = GetTickCount64();
    uint64_t tickNumber = 0;

    while (true) {
        ULONGLONG nowTime = GetTickCount64();
        int ticks = std::min<int>((int)((nowTime - lastTime) / GAME_TICK_MS), 10);
        if (ticks < 1) {
            Sleep(1);
            continue;
        }
        lastTime = nowTime;

        console->clear(WHITE);

        match.draw(*mainRegion);

        while (ticks--) {
            match.doTick(++tickNumber);
        }

        std::ostringstream corner;
        corner << "Tick " << tickNumber;
        printText(console.get(), 0, 0, GAME_WIDTH, 1, corner.str(), BLACK, CENTER, false);

        console->commit();
    }
}
