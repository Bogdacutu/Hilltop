#include "Console.h"
#include "Console.Windows.h"
#include "Game.h"
#include "UI.h"
#include <deque>
#include <iostream>
#include <memory>
#include <sstream>
#include <Windows.h>

#define GAME_TICKS_PER_SEC 20
#define GAME_TICK_MS (1000 / GAME_TICKS_PER_SEC)

using namespace Hilltop::Console;
using namespace Hilltop::Game;
using namespace Hilltop::UI;

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

const std::string FIRE_TEXT = "    ______________  ________\n   / ____/  _/ __ \\/ ____/ /\n  / /_   / // /_/ / __/ / / \n / __/ _/ // _, _/ /___/_/  \n/_/   /___/_/ |_/_____(_)   ";

std::shared_ptr<BufferedConsole> console;

static void preventResizeWindow() {
    HWND window = GetConsoleWindow();
    HMENU menu = GetSystemMenu(window, FALSE);
    DeleteMenu(menu, SC_SIZE, MF_BYCOMMAND);
    DeleteMenu(menu, SC_MAXIMIZE, MF_BYCOMMAND);
    LONG style = GetWindowLong(window, GWL_STYLE);
    SetWindowLong(window, GWL_STYLE, style & ~WS_SIZEBOX);
}

void gameLoop() {
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

    std::shared_ptr<TextBox> tickCounter = TextBox::create();
    tickCounter->x = tickCounter->y = 0;
    tickCounter->width = GAME_WIDTH;
    tickCounter->height = 1;
    tickCounter->color = BLACK;
    tickCounter->alignment = CENTER;

    std::shared_ptr<ElementCollection> bottomArea = ElementCollection::create();
    bottomArea->x = GAME_HEIGHT;
    bottomArea->y = 2;
    bottomArea->width = GAME_WIDTH - 4;
    bottomArea->height = 12;
    bottomArea->drawBackground = true;

    std::shared_ptr<ElementCollection> leftArea = ElementCollection::create();
    leftArea->width = bottomArea->width / 3;
    leftArea->height = bottomArea->height;
    leftArea->x = 0;
    leftArea->y = 0;
    leftArea->backgroundColor = GREEN;
    leftArea->drawBackground = true;
    bottomArea->addChild(*leftArea);

    std::shared_ptr<ElementCollection> rightArea = ElementCollection::create();
    rightArea->width = bottomArea->width / 3;
    rightArea->height = bottomArea->height;
    rightArea->x = 0;
    rightArea->y = bottomArea->width - rightArea->width;
    rightArea->backgroundColor = ORANGE;
    rightArea->drawBackground = true;
    bottomArea->addChild(*rightArea);

    std::shared_ptr<Button> fireButton = Button::create();
    fireButton->width = bottomArea->width - leftArea->width - rightArea->width - 8;
    fireButton->height = bottomArea->height - 4;
    fireButton->x = 2;
    fireButton->y = leftArea->width + 4;
    fireButton->backgroundColor = RED;
    fireButton->color = WHITE;
    fireButton->text = FIRE_TEXT;
    bottomArea->addChild(*fireButton);

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

        std::ostringstream tickCounterText;
        tickCounterText << "Tick " << tickNumber;
        tickCounter->text = tickCounterText.str();
        tickCounter->draw(*console);

        bottomArea->draw(*console);

        console->commit();
    }
}

int main() {
    srand(GetTickCount());

    AttachConsole(-1);
    preventResizeWindow();

    console = WindowsConsole::create(GetStdHandle(STD_OUTPUT_HANDLE), GAME_WIDTH, GAME_HEIGHT + 13);

    gameLoop();
}
