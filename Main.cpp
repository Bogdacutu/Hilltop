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

static void buildGameUI(ElementCollection *bottomArea) {
    std::shared_ptr<ElementCollection> leftArea = ElementCollection::create();
    leftArea->width = bottomArea->width / 3;
    leftArea->height = bottomArea->height;
    leftArea->x = 0;
    leftArea->y = 0;
    bottomArea->addChild(*leftArea);

    std::shared_ptr<ElementCollection> moveArea = ElementCollection::create();
    moveArea->width = leftArea->width - 6;
    moveArea->height = 3;
    moveArea->x = leftArea->height - moveArea->height - 2;
    moveArea->y = 4;
    moveArea->backgroundColor = GRAY;
    moveArea->drawBackground = true;
    leftArea->addChild(*moveArea);

    std::shared_ptr<Button> moveLeft = Button::create();
    moveLeft->width = 5;
    moveLeft->height = 3;
    moveLeft->x = 0;
    moveLeft->y = 0;
    moveLeft->backgroundColor = DARK_GRAY;
    moveLeft->color = GRAY;
    moveLeft->text = "<";
    moveArea->addChild(*moveLeft);

    std::shared_ptr<Button> moveRight = Button::create();
    moveRight->width = 5;
    moveRight->height = 3;
    moveRight->x = 0;
    moveRight->y = moveArea->width - moveRight->width;
    moveRight->backgroundColor = DARK_GRAY;
    moveRight->color = GRAY;
    moveRight->text = ">";
    moveArea->addChild(*moveRight);

    std::shared_ptr<TextBox> moveText = TextBox::create();
    moveText->width = moveArea->width - moveLeft->width - moveRight->width;
    moveText->height = 1;
    moveText->x = 1;
    moveText->y = moveLeft->width;
    moveText->color = BLACK;
    moveText->text = "Moves left: 3";
    moveText->alignment = CENTER;
    moveArea->addChild(*moveText);

    std::shared_ptr<ElementCollection> weaponArea = ElementCollection::create();
    weaponArea->width = leftArea->width - 6;
    weaponArea->height = 3;
    weaponArea->x = 2;
    weaponArea->y = 4;
    weaponArea->backgroundColor = GRAY;
    weaponArea->drawBackground = true;
    leftArea->addChild(*weaponArea);

    std::shared_ptr<Button> weaponLeft = Button::create();
    weaponLeft->width = 5;
    weaponLeft->height = 3;
    weaponLeft->x = 0;
    weaponLeft->y = 0;
    weaponLeft->backgroundColor = DARK_GRAY;
    weaponLeft->color = GRAY;
    weaponLeft->text = "<";
    weaponArea->addChild(*weaponLeft);

    std::shared_ptr<Button> weaponRight = Button::create();
    weaponRight->width = 5;
    weaponRight->height = 3;
    weaponRight->x = 0;
    weaponRight->y = weaponArea->width - weaponRight->width;
    weaponRight->backgroundColor = DARK_GRAY;
    weaponRight->color = GRAY;
    weaponRight->text = ">";
    weaponArea->addChild(*weaponRight);

    std::shared_ptr<Button> weaponIcon = Button::create();
    weaponIcon->width = 2;
    weaponIcon->height = 1;
    weaponIcon->x = 1;
    weaponIcon->y = weaponLeft->width + 2;
    weaponIcon->backgroundColor = BROWN;
    weaponArea->addChild(*weaponIcon);

    std::shared_ptr<TextBox> weaponText = TextBox::create();
    weaponText->width = weaponArea->width - weaponRight->width - weaponIcon->y - weaponIcon->width - 3;
    weaponText->height = 1;
    weaponText->x = 1;
    weaponText->y = weaponIcon->y + weaponIcon->width + 2;
    weaponText->color = BLACK;
    weaponText->text = "Ordinary Missile";
    weaponArea->addChild(*weaponText);

    std::shared_ptr<TextBox> weaponNumber = TextBox::create();
    weaponNumber->width = weaponText->width;
    weaponNumber->height = 1;
    weaponNumber->x = 1;
    weaponNumber->y = weaponText->y;
    weaponNumber->color = DARK_GRAY;
    weaponNumber->text = "3";
    weaponNumber->alignment = RIGHT;
    weaponArea->addChild(*weaponNumber);

    std::shared_ptr<ElementCollection> rightArea = ElementCollection::create();
    rightArea->width = bottomArea->width / 3;
    rightArea->height = bottomArea->height;
    rightArea->x = 0;
    rightArea->y = bottomArea->width - rightArea->width;
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

    std::shared_ptr<ElementCollection> powerArea = ElementCollection::create();
    powerArea->width = rightArea->width - 6;
    powerArea->height = 3;
    powerArea->x = rightArea->height - powerArea->height - 2;
    powerArea->y = 2;
    powerArea->backgroundColor = DARK_GRAY;
    powerArea->drawBackground = true;
    rightArea->addChild(*powerArea);

    std::shared_ptr<ProgressBar> powerBar = ProgressBar::create();
    powerBar->width = powerArea->width;
    powerBar->height = 3;
    powerBar->x = 0;
    powerBar->y = 0;
    powerBar->color = RED;
    powerArea->addChild(*powerBar);

    std::shared_ptr<Button> powerBgText = Button::create();
    powerBgText->width = powerArea->width;
    powerBgText->height = 1;
    powerBgText->x = 1;
    powerBgText->y = 0;
    powerBgText->backgroundColor = GRAY;
    powerBgText->color = DARK_GRAY;
    powerBgText->text = std::string(" <") + std::string(powerArea->width - 4, ' ') + std::string("> ");
    powerArea->addChild(*powerBgText);

    std::shared_ptr<TextBox> powerText = TextBox::create();
    powerText->width = powerArea->width;
    powerText->height = 1;
    powerText->x = 1;
    powerText->y = 0;
    powerText->color = BLACK;
    powerText->text = "Power: 50%";
    powerText->alignment = CENTER;
    powerArea->addChild(*powerText);

    std::shared_ptr<ElementCollection> angleArea = ElementCollection::create();
    angleArea->width = rightArea->width - 6;
    angleArea->height = 3;
    angleArea->x = 2;
    angleArea->y = 2;
    angleArea->backgroundColor = DARK_GRAY;
    angleArea->drawBackground = true;
    rightArea->addChild(*angleArea);

    std::shared_ptr<ProgressBar> angleProgressTop = ProgressBar::create();
    angleProgressTop->width = angleArea->width;
    angleProgressTop->height = 1;
    angleProgressTop->x = 0;
    angleProgressTop->y = 0;
    angleProgressTop->color = RED;
    angleProgressTop->inverted = true;
    angleProgressTop->value = 0.5f;
    angleArea->addChild(*angleProgressTop);

    std::shared_ptr<ProgressBar> angleProgressBottom = ProgressBar::create();
    angleProgressBottom->width = angleArea->width;
    angleProgressBottom->height = 1;
    angleProgressBottom->x = 2;
    angleProgressBottom->y = 0;
    angleProgressBottom->color = RED;
    angleProgressBottom->value = 0.0f;
    angleArea->addChild(*angleProgressBottom);

    std::shared_ptr<Button> angleBgText = Button::create();
    angleBgText->width = angleArea->width;
    angleBgText->height = 1;
    angleBgText->x = 1;
    angleBgText->y = 0;
    angleBgText->backgroundColor = GRAY;
    angleBgText->color = DARK_GRAY;
    angleBgText->text = std::string(" <") + std::string(angleArea->width - 4, ' ') + std::string("> ");
    angleArea->addChild(*angleBgText);

    std::shared_ptr<TextBox> angleText = TextBox::create();
    angleText->width = angleArea->width;
    angleText->height = 1;
    angleText->x = 1;
    angleText->y = 0;
    angleText->color = BLACK;
    angleText->text = "Angle: 90";
    angleText->alignment = CENTER;
    angleArea->addChild(*angleText);
}

static void gameLoop() {
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

    buildGameUI(bottomArea.get());

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
