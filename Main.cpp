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

const unsigned short MENU_WIDTH = 100;
const unsigned short MENU_HEIGHT = 25;

std::shared_ptr<BufferedConsole> console;


std::shared_ptr<ElementCollection> bottomArea;

std::shared_ptr<ElementCollection> weaponArea;

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
bool exitMatch = false;

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

static void tickLoop(std::function<void()> tick, std::function<bool()> loop) {
    ULONGLONG lastTime = GetTickCount64();

    while (true) {
        int ticks = std::min<int>((int)((GetTickCount64() - lastTime) / GAME_TICK_MS), 10);
        if (ticks < 1) {
            Sleep(1);
            continue;
        }

        while (ticks--) {
            if (tick)
                tick();
        }

        if (!loop())
            break;

        lastTime = GetTickCount64();
    }
}

static void callWithNewConsole(std::function<void()> func) {
    std::shared_ptr<BufferedConsole> oldConsole = console;
    console.reset();
    func();
    console = oldConsole;

    std::shared_ptr<BufferedConsole> real = oldConsole;
    std::shared_ptr<SnapshotConsole> snap;
    do {
        snap = std::dynamic_pointer_cast<SnapshotConsole>(real);
        if (snap)
            real = snap->console;
    } while (snap);

    std::shared_ptr<BufferedNativeConsole> native = std::dynamic_pointer_cast<BufferedNativeConsole>(real);
    if (native)
        native->configure();
}

static void callWithConsoleSnapshot(std::function<void()> func) {
    std::shared_ptr<BufferedConsole> oldConsole = console;
    callWithNewConsole([oldConsole, func]() {
        console = SnapshotConsole::create(*oldConsole);
        func();
        console->clear(BLACK);
    });
}

enum {
    PAUSED_RESUME_OPTION,
    PAUSED_LOAD_GAME_OPTION,
    PAUSED_SAVE_GAME_OPTION,
    PAUSED_QUIT_TO_MENU_OPTION,
    PAUSED_EXIT_GAME_OPTION,
    NUM_PAUSED_AREAS
} PauseScreenArea;

static void drawPauseHeader(Element &pauseArea) {
    std::shared_ptr<BufferedConsoleRegion> sub = BufferedConsoleRegion::create(*console,
        pauseArea.width - 4, 3, pauseArea.x, pauseArea.y + 2);

    for (int i = 0; i < sub->width; i++)
        sub->set(0, i, L'▄', make_color(WHITE, BLACK));
    for (int i = 0; i < sub->width; i++)
        sub->set(2, i, L'▄', make_color(BLACK, WHITE));
}

static void pauseScreen() {
    static const int PAUSE_WIDTH = 31;
    static const int PAUSE_HEIGHT = 21;

    std::shared_ptr<ElementCollection> pauseArea = ElementCollection::create();
    pauseArea->width = PAUSE_WIDTH;
    pauseArea->height = PAUSE_HEIGHT;
    pauseArea->x = (console->height - pauseArea->height) / 2;
    pauseArea->y = (console->width - pauseArea->width) / 2;
    pauseArea->backgroundColor = WHITE;
    pauseArea->drawBackground = true;

    std::shared_ptr<Button> pauseHeader = Button::create();
    pauseHeader->width = pauseArea->width - 4;
    pauseHeader->height = 1;
    pauseHeader->x = 1;
    pauseHeader->y = 2;
    pauseHeader->backgroundColor = BLACK;
    pauseHeader->color = WHITE;
    pauseHeader->text = "-- Paused --";
    pauseArea->addChild(*pauseHeader);

    std::shared_ptr<ElementCollection> pauseInner = ElementCollection::create();
    pauseInner->width = pauseArea->width - 4;
    pauseInner->height = pauseArea->height - 4;
    pauseInner->x = 3;
    pauseInner->y = 2;
    pauseInner->backgroundColor = BLACK;
    pauseInner->drawBackground = true;
    pauseArea->addChild(*pauseInner);

    std::shared_ptr<TextBox> resumeOption = TextBox::create();
    resumeOption->width = pauseInner->width - 8;
    resumeOption->height = 1;
    resumeOption->x = 2;
    resumeOption->y = 4;
    resumeOption->text = "Resume game";
    resumeOption->alignment = CENTER;
    pauseInner->addChild(*resumeOption);

    std::shared_ptr<TextBox> loadGameOption = TextBox::create();
    loadGameOption->width = pauseInner->width - 8;
    loadGameOption->height = 1;
    loadGameOption->x = 5;
    loadGameOption->y = 4;
    loadGameOption->text = "Load game";
    loadGameOption->alignment = CENTER;
    pauseInner->addChild(*loadGameOption);

    std::shared_ptr<TextBox> saveGameOption = TextBox::create();
    saveGameOption->width = pauseInner->width - 8;
    saveGameOption->height = 1;
    saveGameOption->x = 8;
    saveGameOption->y = 4;
    saveGameOption->text = "Save game";
    saveGameOption->alignment = CENTER;
    pauseInner->addChild(*saveGameOption);

    std::shared_ptr<TextBox> quitGameOption = TextBox::create();
    quitGameOption->width = pauseInner->width - 8;
    quitGameOption->height = 1;
    quitGameOption->x = 11;
    quitGameOption->y = 4;
    quitGameOption->text = "Quit to main menu";
    quitGameOption->alignment = CENTER;
    pauseInner->addChild(*quitGameOption);

    std::shared_ptr<TextBox> exitGameOption = TextBox::create();
    exitGameOption->width = pauseInner->width - 8;
    exitGameOption->height = 1;
    exitGameOption->x = 14;
    exitGameOption->y = 4;
    exitGameOption->text = "Exit to desktop";
    exitGameOption->alignment = CENTER;
    pauseInner->addChild(*exitGameOption);

    bool stopPause = false;

    std::shared_ptr<Form> form = std::make_shared<Form>(NUM_PAUSED_AREAS);
    form->elements[PAUSED_RESUME_OPTION] = resumeOption;
    form->elements[PAUSED_LOAD_GAME_OPTION] = loadGameOption;
    form->elements[PAUSED_SAVE_GAME_OPTION] = saveGameOption;
    form->elements[PAUSED_QUIT_TO_MENU_OPTION] = quitGameOption;
    form->elements[PAUSED_EXIT_GAME_OPTION] = exitGameOption;
    Form::configureSimpleForm(*form);
    form->actions[PAUSED_RESUME_OPTION] = [&stopPause](Form::event_args_t e)->bool {
        stopPause = true;
        return true;
    };
    form->actions[PAUSED_QUIT_TO_MENU_OPTION] = [&stopPause](Form::event_args_t e)->bool {
        exitMatch = true;
        stopPause = true;
        return true;
    };
    form->actions[PAUSED_EXIT_GAME_OPTION] = [](Form::event_args_t e)->bool {
        exit(0);
        return true;
    };

    tickLoop([&]() {
        form->tick(true, [&stopPause](Form::event_args_t e) {
            stopPause = true;
        });
    }, [&]()->bool {
        if (stopPause)
            return false;

        console->clear(BLACK);

        pauseArea->draw(*console);
        drawPauseHeader(*pauseArea);
        form->draw(*console, *pauseArea);

        console->commit();
        return true;
    });
}

static bool gameGlobalAction(Form::event_args_t e) {
    if (e.type == Form::KEY) {
        if (e.record.wVirtualKeyCode == VK_ESCAPE) {
            callWithConsoleSnapshot(pauseScreen);
            return true;
        }
    }

    return false;
}

static void buildGameUI(ElementCollection *bottomArea) {
    static const std::string FIRE_TEXT = "    ______________  ________\n   / ____/  _/ __ \\/ ____/ /\n  / /_   / // /_/ / __/ / / \n / __/ _/ // _, _/ /___/_/  \n/_/   /___/_/ |_/_____(_)   ";

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
        e.form->isFocused = false;
        return true;
    }

    std::shared_ptr<TankController> player = match->players[match->currentPlayer];
    player->currentWeapon = (player->currentWeapon + delta) % player->weapons.size();
    if (player->currentWeapon < 0)
        player->currentWeapon += (int)player->weapons.size();

    return delta != 0;
}

static void drawWeaponEntry(BufferedConsole &console, Weapon &weapon, int num, bool active) {
    std::shared_ptr<BufferedConsole> sub = BufferedConsoleRegion::create(console,
        console.width - 10, 1, 0, 5);

    ConsoleColor textC = active ? BLACK : GRAY;
    printText(sub.get(), 0, 6, sub->width - 8, 1, weapon.name, textC);

    std::ostringstream numText;
    numText << num;
    if (num >= TankMatch::UNLIMITED_WEAPON_THRESHOLD)
        numText << '+';

    ConsoleColor numC = active ? DARK_GRAY : GRAY;
    printText(sub.get(), 0, 6, sub->width - 8, 1, numText.str(), numC, RIGHT);

    sub->set(0, 2, weapon.icon[0].ch, weapon.icon[0].color);
    sub->set(0, 3, weapon.icon[1].ch, weapon.icon[1].color);
}

static bool shadowPixel(BufferedConsole &console, unsigned short x, unsigned short y, ConsoleColor from,
    ConsoleColor to) {
    BufferedConsole::pixel_t pixel = console.get(x, y);
    ConsoleColor c = (ConsoleColor)((pixel.color & BACKGROUND_COLOR) >> BACKGROUND_SHIFT);
    if (c == from && pixel.ch == L' ') {
        console.set(x, y, L' ', make_bg_color(to));
        return true;
    } else {
        return false;
    }
}

static void drawWeaponList() {
    std::shared_ptr<TankController> player = match->players[match->currentPlayer];
    int height = (int)player->weapons.size();
    unsigned short x = 0;
    unsigned short y = 0;
    Form::findBounds(weaponArea, bottomArea, &x, &y);
    std::shared_ptr<BufferedConsoleRegion> region = BufferedConsoleRegion::create(*console,
        weaponArea->width + 2, height + 2, x - height - 2, y - 1);
    region->enforceBounds = false;
    region->clear(BLACK);

    for (int i = 0; i < region->height; i++) {
        region->set(i, 0, L' ', make_bg_color(WHITE));
        region->set(i, region->width - 1, L' ', make_bg_color(WHITE));

        shadowPixel(*region, i, -1, WHITE, DARK_GRAY);
        shadowPixel(*region, i, region->width, WHITE, DARK_GRAY);

        shadowPixel(*region, i, -2, WHITE, GRAY);
        shadowPixel(*region, i, region->width + 1, WHITE, GRAY);
    }

    for (int i = 1; i < region->width - 1; i++) {
        region->set(0, i, L'▄', make_color(WHITE, DARK_GRAY));
    }

    for (int i = 0; i <= height; i++) {
        std::shared_ptr<BufferedConsoleRegion> sub = BufferedConsoleRegion::create(*region,
            region->width - 2, 1, i + 1, 1);
        bool active = player->currentWeapon == i;
        ConsoleColor bg = active ? GRAY : DARK_GRAY;
        sub->clear(bg);
        if (i == height)
            break;

        drawWeaponEntry(*sub, *player->weapons[i].first, player->weapons[i].second, active);
    }
}

static void weaponAreaDraw() {
    std::shared_ptr<TankController> player = match->players[match->currentPlayer];
    unsigned short x = 0;
    unsigned short y = 0;
    Form::findBounds(weaponArea, bottomArea, &x, &y);
    std::shared_ptr<BufferedConsole> region = BufferedConsoleRegion::create(*console,
        weaponArea->width, 1, x + 1, y);
    drawWeaponEntry(*region, *player->weapons[player->currentWeapon].first,
        player->weapons[player->currentWeapon].second, true);
}

static bool moveAreaAction(Form::event_args_t e) {
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
        e.form->isFocused = false;
        return true;
    }

    std::shared_ptr<TankController> player = match->players[match->currentPlayer];
    if (delta != 0 && player->movesLeft) {
        player->movesLeft--;
        player->tank->doMove(match.get(), delta);
    }

    return delta != 0;
}

static void moveAreaUpdate() {
    std::ostringstream text;
    text << "Moves left: " << match->players[match->currentPlayer]->movesLeft;
    moveText->text = text.str();
}

static bool angleAreaAction(Form::event_args_t e) {
    static const int DELTA_PER_TICK = 1;

    int delta = 0;
    
    switch (e.record.wVirtualKeyCode) {
    case VK_LEFT:
        delta = DELTA_PER_TICK;
        break;
    case VK_RIGHT:
        delta = -DELTA_PER_TICK;
        break;
    case VK_RETURN:
    case VK_SPACE:
        e.form->isFocused = false;
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
    static const int DELTA_PER_TICK = 1;

    int delta = 0;

    switch (e.record.wVirtualKeyCode) {
    case VK_LEFT:
        delta = -DELTA_PER_TICK;
        break;
    case VK_RIGHT:
        delta = DELTA_PER_TICK;
        break;
    case VK_RETURN:
    case VK_SPACE:
        e.form->isFocused = false;
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
    match->tick();
    e.form->currentPos = FIRE_BUTTON_TOP;
    e.form->isFocused = false;
    return true;
}

static void fireButtonUpdate() {
    ConsoleColor color = match->players[match->currentPlayer]->tank->barrelColor;
    fireButton->backgroundColor = color;
    fireButton->color = is_bright_color(color) ? BLACK : WHITE;
}

static void gameFormUpdate() {
    moveAreaUpdate();
    fireButtonUpdate();
    angleAreaUpdate();
    powerAreaUpdate();
}

static void gameLoop() {
    console = WindowsConsole::create(GetStdHandle(STD_OUTPUT_HANDLE), match->width + 4,
        match->height / 2 + 15);

    std::shared_ptr<BufferedConsoleRegion> mainRegion = BufferedConsoleRegion::create(*console,
        match->width, match->height / 2, 1, 2);

    std::shared_ptr<TextBox> tickCounter = TextBox::create();
    tickCounter->x = tickCounter->y = 0;
    tickCounter->width = match->width + 4;
    tickCounter->height = 1;
    tickCounter->color = BLACK;
    tickCounter->alignment = CENTER;

    bottomArea = ElementCollection::create();
    bottomArea->x = match->height / 2 + 2;
    bottomArea->y = 2;
    bottomArea->width = match->width;
    bottomArea->height = 12;
    bottomArea->backgroundColor = BLACK;
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
    gameForm->currentPos = FIRE_BUTTON_TOP;

    // settle tanks
    match->arrangeTanks();
    do {
        match->tick();
    } while (match->recentUpdatesMattered());
    
    tickLoop([&]() {
        match->tick();
        gameForm->tick(match->isAiming, gameGlobalAction);
    }, [&]()->bool {
        if (exitMatch) {
            exitMatch = false;
            return false;
        }

        console->clear(WHITE);

        match->draw(*mainRegion);

        std::ostringstream tickCounterText;
        tickCounterText << "Tick " << match->tickNumber;
        tickCounter->text = tickCounterText.str();
        tickCounter->draw(*console);

        gameFormUpdate();

        bottomArea->draw(*console);
        weaponAreaDraw();

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
            match->players[match->currentPlayer]->movesLeft = TankController::MOVES_PER_TURN;
            if (match->players[match->currentPlayer]->isHuman) {
                match->isAiming = true;
            } else {
                TankController::applyAI(*match->players[match->currentPlayer]);
                match->fire();
            }
        }

        console->commit();
        return true;
    });
}

static void drawMainMenuBackground(BufferedConsole &console) {
    static const std::string LOGO[] = {
        "XX     XX",
        "XX     XX",
        "XX     XX  XX  XX     XX    XXXXXX   XXXXX   XXXXXX",
        "XX     XX  XX  XX     XX    XXXXXX  XXXXXXX  XXXXXXX",
        "XXXXXXXXX  XX  XX     XX      XX   XXX   XXX XX   XX",
        "XXXXXXXXX  XX  XX     XX      XX   XX     XX XXXXXXX",
        "XX     XX  XX  XX     XX      XX   XXX   XXX XXXXXX",
        "XX     XX  XX  XXXXXX XXXXXX  XX    XXXXXXX  XX",
        "XX     XX  XX  XXXXXX XXXXXX  XX     XXXXX   XX",
    };

    static const int HILL[] = {
        46, 46, 45, 45, 44, 44, 43, 43, 42, 42, 41, 41, 41, 40, 40, 39, 39, 38, 38, 37, 36, 35, 35, 34, 32, 31, 30, 28, 16, 4
    };

    DoublePixelBufferedConsole buffer(console.width, console.height * 2);

    buffer.clear(DARK_BLUE);

    for (int i = 0; i < sizeof(HILL) / sizeof(*HILL); i++)
        for (int j = 0; j < HILL[i]; j++)
            buffer.set(buffer.height - i - 1, buffer.width - HILL[i] + j, DARK_GREEN);

    for (unsigned short i = 0; i < 7; i++)
        buffer.set(buffer.height - sizeof(HILL) / sizeof(*HILL) + 1, buffer.width - 25 + i, RED);
    for (unsigned short i = 0; i < 5; i++)
        buffer.set(buffer.height - sizeof(HILL) / sizeof(*HILL), buffer.width - 24 + i, RED);
    for (unsigned short i = 0; i < 3; i++)
        buffer.set(buffer.height - sizeof(HILL) / sizeof(*HILL) - 3 + i, buffer.width - 25 + i, RED);

    for (int i = 0; i < sizeof(LOGO) / sizeof(*LOGO); i++)
        for (int j = 0; j < LOGO[i].length(); j++)
            if (LOGO[i][j] != ' ')
                buffer.set(i + 5, j + 5, WHITE);

    buffer.commit(console);
}

enum {
    NEW_GAME_OPTION,
    LOAD_GAME_OPTION,
    WEAPON_TEST_OPTION,
    EXIT_GAME_OPTION,
    NUM_MAIN_MENU_AREAS
} MainMenuArea;

static bool weaponTestAction(Form::event_args_t e) {
    match = std::make_shared<TankMatch>(180, 90);
    match->buildMap([](float x) { return 0.5f; });

    std::shared_ptr<Tank> tank = Tank::create(RED);
    match->addEntity(*tank);

    std::shared_ptr<TankController> controller = TankController::create();
    controller->tank = tank;
    match->players.push_back(controller);

    for (const std::shared_ptr<Weapon> &weapon : TankMatch::weapons)
        controller->weapons.push_back(std::make_pair(weapon, TankMatch::UNLIMITED_WEAPON_THRESHOLD));

    callWithNewConsole(gameLoop);
    
    e.form->isFocused = false;
    return true;
}

static bool exitGameAction(Form::event_args_t e) {
    exit(0);
    return true;
}

static void mainMenu() {
    console = WindowsConsole::create(GetStdHandle(STD_OUTPUT_HANDLE), MENU_WIDTH, MENU_HEIGHT);

    std::shared_ptr<ElementCollection> mainMenu = ElementCollection::create();
    mainMenu->x = 10;
    mainMenu->y = 6;
    mainMenu->width = MENU_WIDTH / 2 - 12;
    mainMenu->height = MENU_HEIGHT - 12;

    std::shared_ptr<TextBox> newGameOption = TextBox::create();
    newGameOption->x = 1;
    newGameOption->y = 1;
    newGameOption->width = mainMenu->width - 2;
    newGameOption->height = 1;
    newGameOption->text = "> New Game";
    mainMenu->addChild(*newGameOption);

    std::shared_ptr<TextBox> loadGameOption = TextBox::create();
    loadGameOption->x = 4;
    loadGameOption->y = 1;
    loadGameOption->width = mainMenu->width - 2;
    loadGameOption->height = 1;
    loadGameOption->text = "> Load Game";
    mainMenu->addChild(*loadGameOption);

    std::shared_ptr<TextBox> weaponTestOption = TextBox::create();
    weaponTestOption->x = 7;
    weaponTestOption->y = 1;
    weaponTestOption->width = mainMenu->width - 2;
    weaponTestOption->height = 1;
    weaponTestOption->text = "> Weapon Test";
    mainMenu->addChild(*weaponTestOption);

    std::shared_ptr<TextBox> exitGameOption = TextBox::create();
    exitGameOption->x = 10;
    exitGameOption->y = 1;
    exitGameOption->width = mainMenu->width - 2;
    exitGameOption->height = 1;
    exitGameOption->text = "> Exit Game";
    mainMenu->addChild(*exitGameOption);

    std::shared_ptr<Form> mainMenuForm = std::make_shared<Form>(NUM_MAIN_MENU_AREAS);
    mainMenuForm->elements[NEW_GAME_OPTION] = newGameOption;
    mainMenuForm->elements[LOAD_GAME_OPTION] = loadGameOption;
    mainMenuForm->elements[WEAPON_TEST_OPTION] = weaponTestOption;
    mainMenuForm->elements[EXIT_GAME_OPTION] = exitGameOption;
    Form::configureSimpleForm(*mainMenuForm);
    mainMenuForm->actions[WEAPON_TEST_OPTION] = weaponTestAction;
    mainMenuForm->actions[EXIT_GAME_OPTION] = exitGameAction;

    tickLoop([&]() {
        mainMenuForm->tick();
    }, [&]()->bool {
        drawMainMenuBackground(*console);
        mainMenu->draw(*console);
        mainMenuForm->draw(*console, *mainMenu);

        console->commit();
        return true;
    });
}

int main() {
    srand(GetTickCount());

    TankMatch::initalizeWeapons();

    AttachConsole(-1);
    preventResizeWindow();

    mainMenu();
}
