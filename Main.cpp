#include "Console.h"
#include "Console.Windows.h"
#include "Game.h"
#include "UI.h"
#include <deque>
#include <iostream>
#include <memory>
#include <sstream>
#include <Windows.h>

#undef min
#undef max

#define GAME_TICKS_PER_SEC 20
#define GAME_TICK_MS (1000 / GAME_TICKS_PER_SEC)

using namespace Hilltop::Console;
using namespace Hilltop::Game;
using namespace Hilltop::UI;

const unsigned short MENU_WIDTH = 80;
const unsigned short MENU_HEIGHT = 25;

const unsigned short GAME_WIDTH = 200;
const unsigned short GAME_HEIGHT = 50;

const std::string FIRE_TEXT = "    ______________  ________\n   / ____/  _/ __ \\/ ____/ /\n  / /_   / // /_/ / __/ / / \n / __/ _/ // _, _/ /___/_/  \n/_/   /___/_/ |_/_____(_)   ";

std::shared_ptr<BufferedConsole> console;


std::shared_ptr<ElementCollection> bottomArea;

std::shared_ptr<ElementCollection> weaponArea;
std::shared_ptr<Button> weaponIcon[2];
std::shared_ptr<TextBox> weaponText;
std::shared_ptr<TextBox> weaponNumber;

std::shared_ptr<ElementCollection> moveArea;
std::shared_ptr<TextBox> moveText;

std::shared_ptr<Button> fireButton;

std::shared_ptr<ElementCollection> angleArea;
std::shared_ptr<ProgressBar> angleProgressTop;
std::shared_ptr<ProgressBar> angleProgressBottom;
std::shared_ptr<TextBox> angleText;

std::shared_ptr<ElementCollection> powerArea;
std::shared_ptr<ProgressBar> powerBar;
std::shared_ptr<TextBox> powerText;


std::shared_ptr<TankMatch> match;
std::shared_ptr<Form> gameForm;

enum {
    WEAPON_AREA,
    MOVE_AREA,
    FIRE_BUTTON_TOP,
    FIRE_BUTTON_BOTTOM,
    ANGLE_AREA,
    POWER_AREA,
    NUM_GAME_CONTROL_AREAS
} GameControlArea;

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

    weaponArea = ElementCollection::create();
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

    for (int i = 0; i < 2; i++) {
        weaponIcon[i] = Button::create();
        weaponIcon[i]->width = 1;
        weaponIcon[i]->height = 1;
        weaponIcon[i]->x = 1;
        weaponIcon[i]->y = weaponLeft->width + 2 + i;
        weaponArea->addChild(*weaponIcon[i]);
    }

    weaponText = TextBox::create();
    weaponText->width = weaponArea->width - weaponRight->width - weaponLeft->width - 8;
    weaponText->height = 1;
    weaponText->x = 1;
    weaponText->y = weaponLeft->width + 6;
    weaponText->color = BLACK;
    weaponArea->addChild(*weaponText);

    weaponNumber = TextBox::create();
    weaponNumber->width = weaponText->width;
    weaponNumber->height = 1;
    weaponNumber->x = 1;
    weaponNumber->y = weaponText->y;
    weaponNumber->color = DARK_GRAY;
    weaponNumber->alignment = RIGHT;
    weaponArea->addChild(*weaponNumber);

    moveArea = ElementCollection::create();
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

    moveText = TextBox::create();
    moveText->width = moveArea->width - moveLeft->width - moveRight->width;
    moveText->height = 1;
    moveText->x = 1;
    moveText->y = moveLeft->width;
    moveText->color = BLACK;
    moveText->alignment = CENTER;
    moveArea->addChild(*moveText);

    std::shared_ptr<ElementCollection> rightArea = ElementCollection::create();
    rightArea->width = bottomArea->width / 3;
    rightArea->height = bottomArea->height;
    rightArea->x = 0;
    rightArea->y = bottomArea->width - rightArea->width;
    bottomArea->addChild(*rightArea);

    fireButton = Button::create();
    fireButton->width = bottomArea->width - leftArea->width - rightArea->width - 8;
    fireButton->height = bottomArea->height - 4;
    fireButton->x = 2;
    fireButton->y = leftArea->width + 4;
    fireButton->backgroundColor = RED;
    fireButton->color = WHITE;
    fireButton->text = FIRE_TEXT;
    bottomArea->addChild(*fireButton);

    angleArea = ElementCollection::create();
    angleArea->width = rightArea->width - 6;
    angleArea->height = 3;
    angleArea->x = 2;
    angleArea->y = 2;
    angleArea->backgroundColor = DARK_GRAY;
    angleArea->drawBackground = true;
    rightArea->addChild(*angleArea);

    angleProgressTop = ProgressBar::create();
    angleProgressTop->width = angleArea->width;
    angleProgressTop->height = 1;
    angleProgressTop->x = 0;
    angleProgressTop->y = 0;
    angleProgressTop->color = RED;
    angleProgressTop->inverted = true;
    angleProgressTop->value = 0.5f;
    angleArea->addChild(*angleProgressTop);

    angleProgressBottom = ProgressBar::create();
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
    angleBgText->text = std::string("  <") + std::string(angleArea->width - 6, ' ') + std::string(">  ");
    angleArea->addChild(*angleBgText);

    angleText = TextBox::create();
    angleText->width = angleArea->width;
    angleText->height = 1;
    angleText->x = 1;
    angleText->y = 0;
    angleText->color = BLACK;
    angleText->alignment = CENTER;
    angleArea->addChild(*angleText);

    powerArea = ElementCollection::create();
    powerArea->width = rightArea->width - 6;
    powerArea->height = 3;
    powerArea->x = rightArea->height - powerArea->height - 2;
    powerArea->y = 2;
    powerArea->backgroundColor = DARK_GRAY;
    powerArea->drawBackground = true;
    rightArea->addChild(*powerArea);

    powerBar = ProgressBar::create();
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
    powerBgText->text = std::string("  <") + std::string(powerArea->width - 6, ' ') + std::string(">  ");
    powerArea->addChild(*powerBgText);

    powerText = TextBox::create();
    powerText->width = powerArea->width;
    powerText->height = 1;
    powerText->x = 1;
    powerText->y = 0;
    powerText->color = BLACK;
    powerText->alignment = CENTER;
    powerArea->addChild(*powerText);
}

static bool weaponAreaAction(Form::event_args_t e) {
    int delta = 0;

    switch (e.record.wVirtualKeyCode) {
    case VK_LEFT:
    case VK_UP:
        delta = -1;
        break;
    case VK_RIGHT:
    case VK_DOWN:
        delta = 1;
        break;
    case VK_RETURN:
    case VK_SPACE:
        e.form.isFocused = false;
        return true;
    }

    std::shared_ptr<TankController> player = match->players[match->currentPlayer];
    player->currentWeapon = std::min(std::max(0, player->currentWeapon + delta),
        (int)player->weapons.size() - 1);

    return delta != 0;
}

static void drawWeaponEntry(BufferedConsole &console, Weapon &weapon, int num, bool active) {
    std::shared_ptr<BufferedConsole> sub = BufferedConsoleRegion::create(console,
        console.width - 10, 1, 0, 5);

    std::ostringstream numText;
    numText << num;
    ConsoleColor textC = active ? BLACK : GRAY;
    printText(sub.get(), 0, 6, sub->width - 8, 1, weapon.name, textC);

    ConsoleColor numC = active ? DARK_GRAY : GRAY;
    printText(sub.get(), 0, 6, sub->width - 8, 1, numText.str(), numC, RIGHT);

    sub->set(0, 2, weapon.icon[0].ch, weapon.icon[0].color);
    sub->set(0, 3, weapon.icon[1].ch, weapon.icon[1].color);
}

static void drawWeaponList() {
    std::shared_ptr<TankController> player = match->players[match->currentPlayer];
    int height = (int)player->weapons.size();
    unsigned short x = 0;
    unsigned short y = 0;
    Form::findBounds(weaponArea, bottomArea, &x, &y);
    std::shared_ptr<BufferedConsole> region = BufferedConsoleRegion::create(*console,
        weaponArea->width + 2, height + 2, x - height - 2, y - 1);
    region->clear(BLACK);

    for (int i = 0; i < region->height; i++) {
        region->set(i, 0, L' ', make_bg_color(WHITE));
        region->set(i, region->width - 1, L' ', make_bg_color(WHITE));
    }

    for (int i = 1; i < region->width - 1; i++) {
        region->BufferedConsole::set(0, i, L'▄', make_color(WHITE, DARK_GRAY));
    }

    for (int i = 0; i <= height; i++) {
        std::shared_ptr<BufferedConsole> sub = BufferedConsoleRegion::create(*region,
            region->width - 2, 1, i + 1, 1);
        bool active = player->currentWeapon == i;
        ConsoleColor bg = active ? GRAY : DARK_GRAY;
        sub->clear(bg);
        if (i == height)
            break;

        drawWeaponEntry(*sub, *player->weapons[i].first, player->weapons[i].second, active);
    }
}

static void weaponAreaUpdate() {
    std::shared_ptr<TankController> player = match->players[match->currentPlayer];
    weaponText->text = player->weapons[player->currentWeapon].first->name;
    std::ostringstream number;
    number << player->weapons[player->currentWeapon].second;
    weaponNumber->text = number.str();
}

static bool moveAreaAction(Form::event_args_t e) {
    return false;
}

static void moveAreaUpdate() {
    std::ostringstream text;
    text << "Moves left: " << match->players[match->currentPlayer]->movesLeft;
    moveText->text = text.str();
}

static bool angleAreaAction(Form::event_args_t e) {
    static const int deltaPerTick = 1;

    int delta = 0;
    
    switch (e.record.wVirtualKeyCode) {
    case VK_LEFT:
        delta = deltaPerTick;
        break;
    case VK_RIGHT:
        delta = -deltaPerTick;
        break;
    case VK_RETURN:
    case VK_SPACE:
        e.form.isFocused = false;
        return true;
    }

    int angle = match->players[match->currentPlayer]->tank->angle;
    angle = (angle + delta) % 360;
    if (angle < 0)
        angle += 360;
    match->players[match->currentPlayer]->tank->angle = angle;

    return delta != 0;
}

static void angleAreaUpdate() {
    int angle = match->players[match->currentPlayer]->tank->angle;
    std::ostringstream text;
    text << "Angle: " << angle;
    angleText->text = text.str();
    angleProgressTop->value = (float)std::min(180, angle) / 180.0f;
    angleProgressBottom->value = (float)std::max(0, angle - 180) / 179.0f;

    ConsoleColor color = match->players[match->currentPlayer]->tank->color;
    angleProgressTop->color = color;
    angleProgressBottom->color = color;
}

static bool powerAreaAction(Form::event_args_t e) {
    static const int deltaPerTick = 1;

    int delta = 0;

    switch (e.record.wVirtualKeyCode) {
    case VK_LEFT:
        delta = -deltaPerTick;
        break;
    case VK_RIGHT:
        delta = deltaPerTick;
        break;
    case VK_RETURN:
    case VK_SPACE:
        e.form.isFocused = false;
        return true;
    }

    int power = match->players[match->currentPlayer]->tank->power;
    power = std::min(std::max(0, power + delta), 100);
    match->players[match->currentPlayer]->tank->power = power;

    return delta != 0;
}

static void powerAreaUpdate() {
    int power = match->players[match->currentPlayer]->tank->power;
    std::ostringstream text;
    text << "Power: " << power << "%";
    powerText->text = text.str();
    powerBar->value = (float)power / 100.0f;

    ConsoleColor color = match->players[match->currentPlayer]->tank->color;
    powerBar->color = color;
}

static bool fireButtonAction(Form::event_args_t e) {
    match->fire();
    match->isAiming = false;
    match->doTick();
    e.form.currentPos = 0;
    e.form.isFocused = false;
    return true;
}

static void fireButtonUpdate() {
    ConsoleColor color = match->players[match->currentPlayer]->tank->barrelColor;
    fireButton->backgroundColor = color;
    fireButton->color = is_bright_color(color) ? BLACK : WHITE;
}

static void gameFormUpdate() {
    weaponAreaUpdate();
    moveAreaUpdate();
    fireButtonUpdate();
    angleAreaUpdate();
    powerAreaUpdate();
}

static void gameLoop() {
    match = std::make_shared<TankMatch>(GAME_WIDTH - 4, GAME_HEIGHT * 2 - 4);
    match->buildMap([](float x) { return (std::sinf(x * 2 - 1.4f) + 1) / 2 + 0.1f; });

    for (int i = 0; i < 2; i++) {
        std::shared_ptr<Tank> tank = Tank::create(i == 0 ? BLUE : RED);
        match->addEntity(*tank);

        std::shared_ptr<TankController> controller = TankController::create();
        controller->tank = tank;
        controller->isHuman = i == 0;
        controller->team = i + 1;
        match->players.push_back(controller);

        for (const std::shared_ptr<Weapon> &weapon : TankMatch::weapons) {
            controller->weapons.push_back(std::make_pair(weapon, 1));
        }
    }
    match->arrangeTanks();

    std::shared_ptr<BufferedConsoleRegion> mainRegion = BufferedConsoleRegion::create(*console,
        GAME_WIDTH - 2, GAME_HEIGHT - 1, 1, 2);

    ULONGLONG lastTime = GetTickCount64();

    std::shared_ptr<TextBox> tickCounter = TextBox::create();
    tickCounter->x = tickCounter->y = 0;
    tickCounter->width = GAME_WIDTH;
    tickCounter->height = 1;
    tickCounter->color = BLACK;
    tickCounter->alignment = CENTER;

    bottomArea = ElementCollection::create();
    bottomArea->x = GAME_HEIGHT;
    bottomArea->y = 2;
    bottomArea->width = GAME_WIDTH - 4;
    bottomArea->height = 12;
    bottomArea->drawBackground = true;

    buildGameUI(bottomArea.get());

    gameForm = std::make_shared<Form>(NUM_GAME_CONTROL_AREAS);
    gameForm->elements[WEAPON_AREA] = weaponArea;
    gameForm->elements[MOVE_AREA] = moveArea;
    gameForm->elements[FIRE_BUTTON_TOP] = fireButton;
    gameForm->elements[FIRE_BUTTON_BOTTOM] = fireButton;
    gameForm->elements[ANGLE_AREA] = angleArea;
    gameForm->elements[POWER_AREA] = powerArea;
    gameForm->mapping[WEAPON_AREA].bottom = MOVE_AREA;
    gameForm->mapping[WEAPON_AREA].right = FIRE_BUTTON_TOP;
    gameForm->mapping[MOVE_AREA].top = WEAPON_AREA;
    gameForm->mapping[MOVE_AREA].right = FIRE_BUTTON_BOTTOM;
    gameForm->mapping[FIRE_BUTTON_TOP].bottom = FIRE_BUTTON_BOTTOM;
    gameForm->mapping[FIRE_BUTTON_TOP].left = WEAPON_AREA;
    gameForm->mapping[FIRE_BUTTON_TOP].right = ANGLE_AREA;
    gameForm->mapping[FIRE_BUTTON_BOTTOM].top = FIRE_BUTTON_TOP;
    gameForm->mapping[FIRE_BUTTON_BOTTOM].left = MOVE_AREA;
    gameForm->mapping[FIRE_BUTTON_BOTTOM].right = POWER_AREA;
    gameForm->mapping[ANGLE_AREA].bottom = POWER_AREA;
    gameForm->mapping[ANGLE_AREA].left = FIRE_BUTTON_TOP;
    gameForm->mapping[POWER_AREA].top = ANGLE_AREA;
    gameForm->mapping[POWER_AREA].left = FIRE_BUTTON_BOTTOM;
    gameForm->actions[WEAPON_AREA] = weaponAreaAction;
    gameForm->actions[MOVE_AREA] = moveAreaAction;
    gameForm->actions[FIRE_BUTTON_TOP] = fireButtonAction;
    gameForm->actions[FIRE_BUTTON_BOTTOM] = fireButtonAction;
    gameForm->actions[ANGLE_AREA] = angleAreaAction;
    gameForm->actions[POWER_AREA] = powerAreaAction;

    // settle tanks
    do {
        match->doTick();
    } while (match->recentUpdatesMattered());
    
    while (true) {
        ULONGLONG nowTime = GetTickCount64();
        int ticks = std::min<int>((int)((nowTime - lastTime) / GAME_TICK_MS), 10);
        if (ticks < 1) {
            Sleep(1);
            continue;
        }
        lastTime = nowTime;

        console->clear(WHITE);

        match->draw(*mainRegion);

        while (ticks--) {
            match->doTick();
            if (!match->isAiming) {
                Form::drainInputQueue();
            } else {
                gameForm->tick();
            }
        }

        std::ostringstream tickCounterText;
        tickCounterText << "Tick " << match->tickNumber;
        tickCounter->text = tickCounterText.str();
        tickCounter->draw(*console);

        gameFormUpdate();

        bottomArea->draw(*console);

        if (match->isAiming) {
            gameForm->draw(*console, *bottomArea);
            if (gameForm->currentPos == WEAPON_AREA && gameForm->isFocused)
                drawWeaponList();
        } else if (!match->recentUpdatesMattered()) {
            std::shared_ptr<TankController> player = match->players[match->currentPlayer];
            if (player->weapons[player->currentWeapon].second <= 0) {
                player->weapons.erase(player->weapons.begin() + player->currentWeapon);
                player->currentWeapon = 0;
            }

            match->currentPlayer = match->getNextPlayer();
            if (match->players[match->currentPlayer]->isHuman) {
                match->isAiming = true;
            } else {
                TankController::applyAI(*match->players[match->currentPlayer]);
                match->fire();
            }
        }

        console->commit();
    }
}

int main() {
    srand(GetTickCount());

    TankMatch::initalizeWeapons();

    AttachConsole(-1);
    preventResizeWindow();

    console = WindowsConsole::create(GetStdHandle(STD_OUTPUT_HANDLE), GAME_WIDTH, GAME_HEIGHT + 13);

    gameLoop();
}
