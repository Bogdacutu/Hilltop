#include "Console.h"
#include "Console.Windows.h"
#include "UI.h"
#include <boost/archive/iterators/base64_from_binary.hpp>
#include <boost/archive/iterators/binary_from_base64.hpp>
#include <boost/archive/iterators/transform_width.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/export.hpp>
#include <deque>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>

#include <Windows.h>
#undef min
#undef max
#define VK_A 0x41


#include "Game.h"

BOOST_CLASS_EXPORT(Hilltop::Game::Entity)
BOOST_CLASS_EXPORT(Hilltop::Game::BotAttempt)
BOOST_CLASS_EXPORT(Hilltop::Game::SimpleRocket)
BOOST_CLASS_EXPORT(Hilltop::Game::SimpleTrailedRocket)
BOOST_CLASS_EXPORT(Hilltop::Game::GroundTrailedRocket)
BOOST_CLASS_EXPORT(Hilltop::Game::BouncyTrailedRocket)
BOOST_CLASS_EXPORT(Hilltop::Game::RocketTrail)
BOOST_CLASS_EXPORT(Hilltop::Game::Explosion)
BOOST_CLASS_EXPORT(Hilltop::Game::Tracer)
BOOST_CLASS_EXPORT(Hilltop::Game::TankWheel)
BOOST_CLASS_EXPORT(Hilltop::Game::Tank)
BOOST_CLASS_EXPORT(Hilltop::Game::Drop)
BOOST_CLASS_EXPORT(Hilltop::Game::HealthDrop)
BOOST_CLASS_EXPORT(Hilltop::Game::ArmorDrop)
BOOST_CLASS_EXPORT(Hilltop::Game::WeaponDrop)
BOOST_CLASS_EXPORT(Hilltop::Game::ParticleBomb)
BOOST_CLASS_EXPORT(Hilltop::Game::BulletRainCloud)

BOOST_CLASS_EXPORT(Hilltop::Game::Weapon)
BOOST_CLASS_EXPORT(Hilltop::Game::RocketWeapon)
BOOST_CLASS_EXPORT(Hilltop::Game::DirtRocketWeapon)
BOOST_CLASS_EXPORT(Hilltop::Game::GroundRocketWeapon)
BOOST_CLASS_EXPORT(Hilltop::Game::BouncyRocketWeapon)
BOOST_CLASS_EXPORT(Hilltop::Game::ParticleBombWeapon)
BOOST_CLASS_EXPORT(Hilltop::Game::BulletRainWeapon)


#define GAME_TICKS_PER_SEC 20
#define GAME_TICK_MS (1000 / GAME_TICKS_PER_SEC)

using namespace Hilltop::Console;
using namespace Hilltop::Game;
using namespace Hilltop::UI;

const unsigned short MENU_WIDTH = 100;
const unsigned short MENU_HEIGHT = 25;

std::shared_ptr<BufferedConsole> console;


const int START_WEAPONS = 6;
const int MIN_WEAPONS = 2;


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
bool reenterMatch = false;


std::shared_ptr<TextBox> playerText[4];
std::shared_ptr<TextBox> playerType[4];
std::shared_ptr<TextBox> playerTeam[4];
std::shared_ptr<TextBox> playerTank[4];
std::shared_ptr<TextBox> newGameStartText;


const wchar_t fileDialogFilter[] = L"Hilltop Map (*.htm)\0*.htm\0All Files\0*.*\0\0";
wchar_t fileDialogBuffer[MAX_PATH] = {};
std::wstring chosenFilename;


static OPENFILENAME buildOpenFileName() {
    OPENFILENAME open = { sizeof(OPENFILENAME) };
    open.lpstrFile = fileDialogBuffer;
    open.nMaxFile = sizeof(fileDialogBuffer) / sizeof(*fileDialogBuffer);
    open.lpstrFilter = fileDialogFilter;
    open.lpstrDefExt = L"htm";
    return open;
}

static bool askLoadFile() {
    OPENFILENAME open = buildOpenFileName();
    open.Flags |= OFN_FILEMUSTEXIST;
    if (GetOpenFileName(&open)) {
        chosenFilename = fileDialogBuffer;
        return true;
    } else {
        return false;
    }
}

static bool askSaveFile() {
    OPENFILENAME open = buildOpenFileName();
    open.Flags |= OFN_OVERWRITEPROMPT;
    if (GetSaveFileName(&open)) {
        chosenFilename = fileDialogBuffer;
        return true;
    } else {
        return false;
    }
}

enum GameControlArea {
    WEAPON_AREA,
    MOVE_AREA,
    FIRE_BUTTON_TOP,
    FIRE_BUTTON_BOTTOM,
    ANGLE_AREA,
    POWER_AREA,
    NUM_GAME_CONTROL_AREAS
};

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

void messageBoxImpl(std::string text, std::string title) {
    int left = console->width / 4;
    int right = console->width - left;
    int width = right - left;
    TextBoxSize size = printText(nullptr, 0, 0, width, console->height, text, WHITE);
    int top = std::max(0, (console->height - size.lines - 4) / 2);
    width = std::max<int>(size.cols, (int)title.length()) + 12;
    left = (console->width - width) / 2;
    int height = size.lines + 4;
    top = (console->height - height) / 2;
    std::shared_ptr<BufferedConsoleRegion> sub = BufferedConsoleRegion::create(*console,
        width, height, top, left);
    std::shared_ptr<BufferedConsoleRegion> subSub = BufferedConsoleRegion::create(*sub,
        sub->width - 4, sub->height - 2, 1, 2);
    std::shared_ptr<Form> form = std::make_shared<Form>(1);
    bool exit = false;

    tickLoop([&]() {
        form->tick(false, [&](Form::event_args_t e) {
            if (e.type == Form::KEY) {
                switch (e.record.wVirtualKeyCode) {
                case VK_RETURN:
                case VK_SPACE:
                case VK_ESCAPE:
                    exit = true;
                    break;
                }
            }
        });
    }, [&]()->bool {
        if (exit)
            return false;

        sub->clear(WHITE);
        subSub->clear(BLACK);

        if (title.length())
            printText(sub.get(), 0, 0, sub->width, 1, "- " + title + " -", BLACK, CENTER, false);
        printText(subSub.get(), 1, 1, subSub->width - 2, subSub->height - 2, text, GRAY, CENTER);
        
        console->commit();
        return true;
    });
}

void messageBox(std::string text, std::string title = "") {
    callWithConsoleSnapshot([text, title]() {
        messageBoxImpl(text, title);
    });
}

static void saveGame() {
    static const std::string HTML_BEGIN = "<html><head><title>Hilltop Save File</title><style>body{text-align:center;font-family:Verdana,sans-serif;font-size:18pt}div{margin:20px;display:inline-block}span{font-size:0;display:block;margin:0;padding:0}b,i{display:inline-block;width:10px;height:10px}i{background-color:white}a{font-size:11pt;position:relative;top:-4pt;color:blue}pre{color:#ccc;white-space:normal;word-wrap:break-word;max-width:600px;font-size:7pt;margin-left:auto;margin-right:auto;text-align:justify}</style></head><body onload='document.getElementsByTagName(\"pre\")[0].innerHTML=atob(document.childNodes[1].textContent.trim())'><br>";
    static const std::string TANK_COMMON = "<span><i></i><b></b><b></b><b></b><b></b><b></b><i></i></span><span><b></b><b></b><b></b><b></b><b></b><b></b><b></b></span>";
    static const std::string TANK_LEFT = "<span><b></b><i></i><i></i><i></i><i></i><i></i><i></i></span><span><i></i><b></b><i></i><i></i><i></i><i></i><i></i></span><span><i></i><i></i><b></b><i></i><i></i><i></i><i></i></span>" + TANK_COMMON;
    static const std::string TANK_RIGHT = "<span><i></i><i></i><i></i><i></i><i></i><i></i><b></b></span><span><i></i><i></i><i></i><i></i><i></i><b></b><i></i></span><span><i></i><i></i><i></i><i></i><b></b><i></i><i></i></span>" + TANK_COMMON;
    static const std::string HTML_END = "<br><br>This is a save file for Hilltop.<br>Load it to continue your match.<br><br>Hilltop is a tank artillery game.<br>Download Hilltop at:<br><a href='https://github.com/Bogdacutu/Hilltop'>https://github.com/Bogdacutu/Hilltop</a><br><br><br><pre></pre></body></html>";

    using namespace boost::archive::iterators;
    typedef base64_from_binary<transform_width<std::string::iterator, 6, 8>> base64_t;

    if (!askSaveFile())
        return;
    std::ofstream fout(chosenFilename);
    chosenFilename.clear();

    std::ostringstream ss;
    boost::archive::text_oarchive archive(ss);
    archive << match;

    fout << "<!DOCTYPE html><!-- ";
    {
        std::string data = ss.str();
        std::copy(base64_t(data.begin()), base64_t(data.end()), std::ostream_iterator<char>(fout));
        fout << std::string((3 - data.size() % 3) % 3, '=');
        ss.clear();
    }
    fout << " -->";
    
    std::vector<int> players;
    for (int i = 0; i < match->players.size(); i++)
        players.push_back(i);
    std::sort(players.begin(), players.end(), [](int x, int y)->bool {
        return match->players[x]->tank->position.Y < match->players[y]->tank->position.Y;
    });

    fout << HTML_BEGIN;
    for (int i = 0; i < players.size(); i++) {
        std::shared_ptr<TankController> player = match->players[players[i]];

        fout << "<div style='background-color:rgb(";
        COLORREF color = mapWindowsColor(player->tank->getActualColor());
        fout << (int)GetRValue(color) << "," << (int)GetGValue(color) << "," << (int)GetBValue(color);
        fout << ")'>";

        int angle = player->tank->angle;
        bool right = angle < 90 || angle >= 270;
        fout << (right ? TANK_RIGHT : TANK_LEFT);

        fout << "</div>";
    }
    fout << HTML_END << std::endl;
}

static bool loadGame() {
    using namespace boost::archive::iterators;
    typedef transform_width<binary_from_base64<std::string::iterator>, 8, 6> binary_t;
    
    if (!askLoadFile())
        return false;
    std::ifstream fin(chosenFilename);
    chosenFilename.clear();

    fin.ignore(16, ' ');
    fin.ignore(16, ' ');
    std::string str;
    fin >> str;

    try {
        std::string decoded(binary_t(str.begin()), binary_t(str.end()));
        std::istringstream ss(decoded);
        boost::archive::text_iarchive archive(ss);
        archive >> match;
    } catch (std::exception &e) {
        messageBox(e.what(), "Error while loading saved game");
        return false;
    }

    return true;
}

enum PauseScreenArea {
    PAUSED_RESUME_OPTION,
    PAUSED_LOAD_GAME_OPTION,
    PAUSED_SAVE_GAME_OPTION,
    PAUSED_QUIT_TO_MENU_OPTION,
    PAUSED_EXIT_GAME_OPTION,
    NUM_PAUSED_AREAS
};

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
    form->actions[PAUSED_LOAD_GAME_OPTION] = [&stopPause](Form::event_args_t e)->bool {
        unsigned short w, h;
        w = match->width;
        h = match->height;
        if (loadGame()) {
            if (match->width != w || match->height != h) {
                exitMatch = true;
                reenterMatch = true;
            }
            stopPause = true;
        }
        e.form->isFocused = false;
        return true;
    };
    form->actions[PAUSED_SAVE_GAME_OPTION] = [&stopPause](Form::event_args_t e)->bool {
        saveGame();
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
            if (e.type == Form::KEY && e.record.wVirtualKeyCode == VK_ESCAPE)
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
    if (num < TankMatch::UNLIMITED_WEAPON_THRESHOLD)
        numText << num;
    else
        numText << "99+";

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
    case VK_DOWN:
        delta = -DELTA_PER_TICK;
        break;
    case VK_RIGHT:
    case VK_UP:
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
    ConsoleColor color = match->players[match->currentPlayer]->tank->color;
    fireButton->backgroundColor = color;
    fireButton->color = is_bright_color(color) ? BLACK : WHITE;
}

static void gameFormUpdate() {
    moveAreaUpdate();
    fireButtonUpdate();
    angleAreaUpdate();
    powerAreaUpdate();
}

static bool gameGlobalAction(Form::event_args_t e) {
    if (e.type == Form::KEY) {
        switch (e.record.wVirtualKeyCode) {
        case VK_ESCAPE:
            callWithConsoleSnapshot(pauseScreen);
            return true;
        case VK_A - 'A' + 'W':
            e.record.wVirtualKeyCode = VK_UP;
            return powerAreaAction(e);
        case VK_A - 'A' + 'S':
            e.record.wVirtualKeyCode = VK_DOWN;
            return powerAreaAction(e);
        case VK_A - 'A' + 'A':
            e.record.wVirtualKeyCode = VK_LEFT;
            return angleAreaAction(e);
        case VK_A - 'A' + 'D':
            e.record.wVirtualKeyCode = VK_RIGHT;
            return angleAreaAction(e);
        }
    }

    return false;
}

enum PlayerTeam {
    TEAM_RED = 1,
    TEAM_GREEN = 2,
    TEAM_BLUE = 3,
    TEAM_YELLOW = 4,
};

const char *PLAYER_TEAM_NAMES[] = {
    "invalid",
    "Red",
    "Green",
    "Blue",
    "Yellow"
};

static void gameLoop() {
    static const bool SHOW_TICKS = false;

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
    
    tickLoop([&]() {
        if (!match->gameOver) {
            match->tick();
            gameForm->tick(match->isAiming && match->players[match->currentPlayer]->isHuman,
                gameGlobalAction);
        } else {
            gameForm->tick(false, gameGlobalAction);
        }
    }, [&]()->bool {
        if (exitMatch) {
            exitMatch = false;
            return false;
        }

        console->clear(WHITE);

        match->draw(*mainRegion);

        std::string gameOverText;
        if (match->gameOver) {
            for (int i = 0; i < match->players.size(); i++) {
                if (match->players[i]->tank->alive) {
                    gameOverText = PLAYER_TEAM_NAMES[match->players[i]->team];
                    gameOverText += " won!";
                    break;
                }
            }
        }

        std::ostringstream tickCounterText;
        if (match->gameOver)
            tickCounterText << gameOverText;
        else if (match->isAiming && !match->players[match->currentPlayer]->isHuman)
            tickCounterText << "Bot thinking";
        if (SHOW_TICKS) {
            if (tickCounterText.str().size())
                tickCounterText << " - ";
            tickCounterText << "Tick " << match->tickNumber;
        }
        tickCounter->text = tickCounterText.str();
        tickCounter->draw(*console);

        gameFormUpdate();

        bottomArea->draw(*console);
        weaponAreaDraw();

        if (match->isAiming && match->players[match->currentPlayer]->isHuman) {
            gameForm->draw(*console, *bottomArea);
            if (gameForm->currentPos == WEAPON_AREA && gameForm->isFocused)
                drawWeaponList();
        }

        console->commit();
        
        if (match->gameOver) {
            if (!match->shownGameOver) {
                messageBox(gameOverText, "Game over");
                match->shownGameOver = true;
            }
        } else if (match->isAiming) {
            if (!match->players[match->currentPlayer]->isHuman) {
                match->isAiming = !TankController::applyAI(match.get(),
                    *match->players[match->currentPlayer]);
                if (!match->isAiming)
                    match->fire();
            }
        } else if (!match->isAiming && !match->recentUpdatesMattered()) {
            for (int i = 0; i < match->players.size(); i++) {
                std::shared_ptr<TankController> player = match->players[i];
                if (player->weapons[player->currentWeapon].second <= 0) {
                    player->weapons.erase(player->weapons.begin() + player->currentWeapon);
                    player->currentWeapon = 0;
                }
                if (player->getWeaponCount() <= MIN_WEAPONS) {
                    while (player->getWeaponCount() < START_WEAPONS)
                        player->addRandomWeapon();
                }
            }

            int nextPlayer = match->getNextPlayer();
            if (nextPlayer >= 0) {
                match->currentPlayer = nextPlayer;
                match->players[match->currentPlayer]->movesLeft =
                    match->players[match->currentPlayer]->movesPerTurn;
                match->isAiming = true;
                if (rand() % TankMatch::AIRDROP_EVERY_TURNS == 0)
                    match->doAirdrop();
            } else {
                match->gameOver = true;
            }
        }

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

enum NewGameArea {
    TANK_TYPE_OPTION = 0,
    TANK_TEAM_OPTION,
    TANK_CUSTOMIZE_OPTION,
    NUM_PLAYER_NEW_GAME_AREAS,
    GAME_OPTIONS_LEFT = TANK_TYPE_OPTION + NUM_PLAYER_NEW_GAME_AREAS * 4,
    GAME_OPTIONS_RIGHT,
    START_GAME_OPTION,
    NUM_NEW_GAME_AREAS
};

enum BotDifficulty {
    BOT_EASY = 0,
    BOT_MEDIUM,
    BOT_HARD,
    NUM_BOT_DIFFICULTIES
};

const char *BOT_DIFFICULTY_NAMES[] = {
    "Easy",
    "Medium",
    "Hard"
};

enum TankAttribute {
    TANK_HEALTH,
    TANK_STAMINA,
    TANK_DAMAGE,
    TANK_ARMOR,
    NUM_TANK_ATTRIBUTES,
};

const int MIN_TANK_ATTRIBUTE_VALUE = 1;
const int MAX_TANK_ATTRIBUTE_VALUE = 7;
const int DEFAULT_TANK_ATTRIBUTE_VALUE = 4;
const int TANK_ATTRIBUTE_SUM = NUM_TANK_ATTRIBUTES * DEFAULT_TANK_ATTRIBUTE_VALUE;

const char *TANK_ATTRIBUTE_NAMES[] = {
    "Health",
    "Stamina",
    "Damage",
    "Armor"
};

enum MapType {
    MAP_RANDOM,
    MAP_HILLSIDE,
    MAP_HILLTOP
};

const char *MAP_TYPE_NAMES[] = {
    "Random",
    "Hillside",
    "Hilltop"
};

const char *FIRING_MODE_NAMES[] = {
    "Solo",
    "Teams",
    "Everyone"
};

struct {
    struct {
        bool enabled = false;
        bool human = false;
        BotDifficulty difficulty = BOT_MEDIUM;
        int team;
        int tank[NUM_TANK_ATTRIBUTES];
    } players[4];
    MapType mapType = MAP_HILLSIDE;
    TankMatch::FiringMode firingMode = TankMatch::FIRE_AS_TEAM;
} newGameSettings;

int activeTankAttribute = 0;

std::shared_ptr<Form> customizeForm;
std::shared_ptr<TextBox> customizeAttrLabel[NUM_TANK_ATTRIBUTES];
std::shared_ptr<TextBox> customizeControls[NUM_TANK_ATTRIBUTES];

bool exitNewGame = false;

static void updateCustomizeControls(int index) {
    activeTankAttribute = customizeForm->currentPos;

    for (int i = 0; i < NUM_TANK_ATTRIBUTES; i++) {
        ConsoleColor color = GRAY;
        if (i == activeTankAttribute)
            color = YELLOW;
        customizeAttrLabel[i]->color = color;
        customizeControls[i]->color = color;
    }
}

static int getTankAttributeSum(int index) {
    int sum = 0;
    for (int i = 0; i < NUM_TANK_ATTRIBUTES; i++)
        sum += newGameSettings.players[index].tank[i];
    return sum;
}

static void drawTankAttributes(BufferedConsole &console, int index) {
    std::shared_ptr<BufferedConsole> buf = BufferedConsoleRegion::create(console,
        MENU_WIDTH / 2 + 10, MENU_HEIGHT - 12, 10, 5);

    for (int i = 0; i < NUM_TANK_ATTRIBUTES; i++) {
        ConsoleColor color = WHITE;
        if (i == activeTankAttribute)
            color = YELLOW;
        color = make_bg_color(color);
        for (int j = 0; j < newGameSettings.players[index].tank[i]; j++) {
            buf->set(3 + i * 2, 20 + j * 3, L' ', color);
            buf->set(3 + i * 2, 21 + j * 3, L' ', color);
        }
    }
}

static bool customizeFormAction(int index, Form::event_args_t e) {
    if (e.type == Form::KEY) {
        int delta = 0;

        switch (e.record.wVirtualKeyCode) {
        case VK_LEFT:
            delta = -1;
            break;
        case VK_RIGHT:
            delta = 1;
            break;
        }

        int sum = getTankAttributeSum(index);
        if (sum + delta <= TANK_ATTRIBUTE_SUM) {
            int val = newGameSettings.players[index].tank[activeTankAttribute] + delta;
            val = std::min(MAX_TANK_ATTRIBUTE_VALUE, std::max(MIN_TANK_ATTRIBUTE_VALUE, val));
            newGameSettings.players[index].tank[activeTankAttribute] = val;
        }
    }

    e.form->isFocused = false;
    return true;
}

static void customizeTankMenu(int index) {
    activeTankAttribute = 0;

    std::shared_ptr<ElementCollection> tankMenu = ElementCollection::create();
    tankMenu->width = MENU_WIDTH / 2 + 10;
    tankMenu->height = MENU_HEIGHT - 12;
    tankMenu->x = 10;
    tankMenu->y = 5;

    std::shared_ptr<Button> blueBg = Button::create();
    blueBg->width = MENU_WIDTH / 2 - 3;
    blueBg->height = MENU_HEIGHT - 12;
    blueBg->x = 0;
    blueBg->y = 0;
    blueBg->backgroundColor = DARK_BLUE;
    tankMenu->addChild(*blueBg);

    std::shared_ptr<TextBox> title = TextBox::create();
    title->width = tankMenu->width;
    title->height = 1;
    title->x = 0;
    title->y = 0;
    title->color = WHITE;
    title->alignment = CENTER;
    std::ostringstream titleText;
    titleText << "Tank " << (index + 1);
    titleText << " - ";
    titleText << PLAYER_TEAM_NAMES[newGameSettings.players[index].team] << " Team";
    title->text = titleText.str();
    tankMenu->addChild(*title);
    
    for (int i = 0; i < NUM_TANK_ATTRIBUTES; i++) {
        customizeAttrLabel[i] = TextBox::create();
        customizeAttrLabel[i]->width = 10;
        customizeAttrLabel[i]->height = 1;
        customizeAttrLabel[i]->x = 3 + i * 2;
        customizeAttrLabel[i]->y = 0;
        customizeAttrLabel[i]->color = WHITE;
        customizeAttrLabel[i]->alignment = RIGHT;
        customizeAttrLabel[i]->text = TANK_ATTRIBUTE_NAMES[i];
        tankMenu->addChild(*customizeAttrLabel[i]);

        customizeControls[i] = TextBox::create();
        customizeControls[i]->width = 26;
        customizeControls[i]->height = 1;
        customizeControls[i]->x = 3 + i * 2;
        customizeControls[i]->y = 17;
        customizeControls[i]->color = WHITE;
        customizeControls[i]->text = " <" + std::string(22, ' ') + "> ";
        tankMenu->addChild(*customizeControls[i]);
    }

    customizeForm = std::make_shared<Form>(NUM_TANK_ATTRIBUTES);
    for (int i = 0; i < NUM_TANK_ATTRIBUTES; i++) {
        customizeForm->elements[i] = customizeControls[i];

        if (i > 0)
            customizeForm->mapping[i].top = i - 1;
        if (i < NUM_TANK_ATTRIBUTES - 1)
            customizeForm->mapping[i].bottom = i + 1;
        customizeForm->mapping[i].left = customizeForm->mapping[i].right = Form::ACT_WITHOUT_FOCUS;

        customizeForm->actions[i] = [index](Form::event_args_t e)->bool {
            return customizeFormAction(index, e);
        };
    }

    bool exitCustomize = false;

    tickLoop([&]() {
        customizeForm->tick(true, [&](Form::event_args_t e) {
            if (e.type == Form::KEY && e.record.wVirtualKeyCode == VK_ESCAPE)
                exitCustomize = true;
        });
    }, [&]()->bool {
        if (exitCustomize)
            return false;

        console->clear(BLACK);

        updateCustomizeControls(index);

        tankMenu->draw(*console);
        customizeForm->draw(*console, *tankMenu);

        drawTankAttributes(*console, index);

        console->commit();
        return true;
    });
}

std::shared_ptr<TextBox> gameOptionLabels[2];
std::shared_ptr<TextBox> gameOptionOptions[6];

static void updateGameOptionMenu() {
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 3; j++) {
            const int idx = i * 3 + j;
            ConsoleColor color = DARK_GRAY;
            if (i == 0) {
                if (j == newGameSettings.mapType)
                    color = WHITE;
            } else {
                if (j == newGameSettings.firingMode)
                    color = WHITE;
            }
            gameOptionOptions[idx]->color = color;
        }
    }
}

static void gameOptionsMenu() {
    std::shared_ptr<ElementCollection> gameOptions = ElementCollection::create();
    gameOptions->width = MENU_WIDTH / 2 - 3;
    gameOptions->height = MENU_HEIGHT - 12;
    gameOptions->x = 10;
    gameOptions->y = 5;
    gameOptions->backgroundColor = DARK_BLUE;
    gameOptions->drawBackground = true;

    for (int i = 0; i < 2; i++) {
        gameOptionLabels[i] = TextBox::create();
        gameOptionLabels[i]->width = 12;
        gameOptionLabels[i]->height = 1;
        gameOptionLabels[i]->x = 3 + i * 3;
        gameOptionLabels[i]->y = 0;
        gameOptionLabels[i]->color = GRAY;
        gameOptionLabels[i]->alignment = RIGHT;
        if (i == 0)
            gameOptionLabels[i]->text = "Map Type";
        else
            gameOptionLabels[i]->text = "Firing Mode";
        gameOptionLabels[i]->text += ":";
        gameOptions->addChild(*gameOptionLabels[i]);

        for (int j = 0; j < 3; j++) {
            const int idx = i * 3 + j;
            gameOptionOptions[idx] = TextBox::create();
            gameOptionOptions[idx]->width = 8;
            gameOptionOptions[idx]->height = 1;
            gameOptionOptions[idx]->x = 3 + i * 3;
            gameOptionOptions[idx]->y = 16 + 11 * j;
            gameOptionOptions[idx]->color = WHITE;
            gameOptionOptions[idx]->alignment = CENTER;
            if (i == 0)
                gameOptionOptions[idx]->text = MAP_TYPE_NAMES[j];
            else
                gameOptionOptions[idx]->text = FIRING_MODE_NAMES[j];
            gameOptions->addChild(*gameOptionOptions[idx]);
        }
    }

    std::shared_ptr<Form> gameOptionsForm = std::make_shared<Form>(6);
    for (int i = 0; i < 6; i++)
        gameOptionsForm->elements[i] = gameOptionOptions[i];
    Form::configureMatrixForm(*gameOptionsForm, 2, 3);
    for (int i = 0; i < 6; i++) {
        gameOptionsForm->actions[i] = [](Form::event_args_t e)->bool {
            int row = e.position / 3;
            int col = e.position % 3;

            if (row == 0) {
                newGameSettings.mapType = (MapType)col;
            } else {
                newGameSettings.firingMode = (TankMatch::FiringMode)col;
            }

            e.form->isFocused = false;
            return true;
        };
    }

    bool exitGameOptions = false;

    tickLoop([&]() {
        gameOptionsForm->tick(true, [&](Form::event_args_t e) {
            if (e.type == Form::KEY && e.record.wVirtualKeyCode == VK_ESCAPE)
                exitGameOptions = true;
        });
    }, [&]()->bool {
        if (exitGameOptions)
            return false;

        console->clear(BLACK);

        updateGameOptionMenu();

        gameOptions->draw(*console);
        gameOptionsForm->draw(*console, *gameOptions);
        
        console->commit();
        return true;
    });
}

static bool newGameValid() {
    bool haveBot = false;
    for (int i = 0; i < 4; i++) {
        if (newGameSettings.players[i].enabled) {
            if (newGameSettings.players[i].human || haveBot) {
                return true;
            } else {
                haveBot = true;
            }
        }
    }
    return false;
}

static void updateNewGameMenu() {
    for (int i = 0; i < 4; i++) {
        std::string type = "Empty";
        ConsoleColor color = BLACK;
        if (newGameSettings.players[i].enabled) {
            color = WHITE;
            type = newGameSettings.players[i].human ? "Human" :
                BOT_DIFFICULTY_NAMES[newGameSettings.players[i].difficulty];
        }

        playerText[i]->color = newGameSettings.players[i].enabled ? WHITE : GRAY;

        playerType[i]->text = type;
        playerType[i]->color = playerText[i]->color;
        
        playerTeam[i]->text = PLAYER_TEAM_NAMES[newGameSettings.players[i].team];
        playerTeam[i]->color = color;

        playerTank[i]->color = color;
    }

    newGameStartText->color = newGameValid() ? WHITE : BLACK;
}

static void runGameLoop() {
    do {
        reenterMatch = false;
        callWithNewConsole(gameLoop);
    } while (reenterMatch);
}

static bool startGameAction(Form::event_args_t e) {
    if (newGameValid()) {
        exitNewGame = true;

        match = std::make_shared<TankMatch>();

        std::function<float(float)> invert = [](float x)->float { return x; };
        if (scale((float)rand(), 0, RAND_MAX, 0, 1) >= 0.5f)
            invert = [](float x)->float { return 1.0f - x; };

        switch (newGameSettings.mapType) {
        case MAP_RANDOM: {
            float f[4];
            for (int i = 0; i < 4; i++)
                f[i] = scale((float)rand(), 0, RAND_MAX, 1.0f, 10.0f);
            float a[4];
            for (int i = 0; i < 4; i++)
                a[i] = scale((float)rand(), 0, RAND_MAX, -1 / f[i], 1 / f[i]);
            float o[4];
            for (int i = 0; i < 4; i++)
                o[i] = scale((float)rand(), 0, RAND_MAX, 0.0f, 6.28f);
            match->buildMap([invert, &f, &a, &o](float x)->float {
                float ret = 0;
                for (int i = 0; i < 4; i++)
                    ret += a[i] * std::sinf(f[i] * invert(x) + o[i]);
                return 0.5f + ret;
            });
            break;
        }
        case MAP_HILLSIDE:
            match->buildMap([invert](float x)->float {
                return (std::sinf(invert(x) * 2 - 1.4f) + 1.0f) / 2.0f + 0.1f;
            });
            break;
        case MAP_HILLTOP:
            match->buildMap([invert](float x)->float {
                return (std::sinf(invert(x) * 2) + 0.1f) / 1.5f + 0.1f;
            });
            break;
        }

        for (int i = 0; i < 4; i++) {
            if (newGameSettings.players[i].enabled) {
                ConsoleColor color = BLACK;
                switch (newGameSettings.players[i].team) {
                case TEAM_BLUE:
                    color = BLUE;
                    break;
                case TEAM_GREEN:
                    color = GREEN;
                    break;
                case TEAM_RED:
                    color = RED;
                    break;
                case TEAM_YELLOW:
                    color = YELLOW;
                    break;
                }

                std::shared_ptr<Tank> tank = Tank::create(color);
                tank->maxHealth = 60 + 10 * newGameSettings.players[i].tank[TANK_HEALTH];
                tank->health = tank->maxHealth;
                tank->damage = 0.6f + 0.1f * newGameSettings.players[i].tank[TANK_DAMAGE];
                tank->maxArmor = (int)scale((float)newGameSettings.players[i].tank[TANK_ARMOR],
                    (float)MIN_TANK_ATTRIBUTE_VALUE, (float)MAX_TANK_ATTRIBUTE_VALUE, 0, 100);
                tank->armor = tank->maxArmor;
                match->addEntity(*tank);

                std::shared_ptr<TankController> controller = TankController::create();
                controller->tank = tank;
                controller->isHuman = newGameSettings.players[i].human;
                controller->botDifficulty = newGameSettings.players[i].difficulty;
                controller->team = newGameSettings.players[i].team;
                controller->movesPerTurn = 5 + 5 * newGameSettings.players[i].tank[TANK_STAMINA];
                controller->movesLeft = controller->movesPerTurn;
                match->players.push_back(controller);

                for (int i = 0; i < START_WEAPONS; i++)
                    controller->addRandomWeapon();
            }
        }

        match->isAiming = match->players[match->currentPlayer]->isHuman;

        match->firingMode = newGameSettings.firingMode;

        match->arrangeTanks();

        runGameLoop();
    }

    e.form->isFocused = false;
    return true;
}

static void newGameMenu() {
    std::shared_ptr<ElementCollection> newGameMenu = ElementCollection::create();
    newGameMenu->width = MENU_WIDTH / 2 - 4;
    newGameMenu->height = MENU_HEIGHT - 12;
    newGameMenu->x = 10;
    newGameMenu->y = 5;
    newGameMenu->backgroundColor = DARK_BLUE;
    newGameMenu->drawBackground = true;
    
    for (int i = 0; i < 4; i++) {
        playerText[i] = TextBox::create();
        playerText[i]->width = 9;
        playerText[i]->height = 1;
        playerText[i]->x = i * 2 + 1;
        playerText[i]->y = 0;
        std::ostringstream playerTextText;
        playerTextText << "Player " << (i + 1) << ":";
        playerText[i]->text = playerTextText.str();
        newGameMenu->addChild(*playerText[i]);

        playerType[i] = TextBox::create();
        playerType[i]->width = 6;
        playerType[i]->height = 1;
        playerType[i]->x = i * 2 + 1;
        playerType[i]->y = playerText[i]->y + playerText[i]->width + 4;
        playerType[i]->alignment = CENTER;
        newGameMenu->addChild(*playerType[i]);

        playerTeam[i] = TextBox::create();
        playerTeam[i]->width = 7;
        playerTeam[i]->height = 1;
        playerTeam[i]->x = i * 2 + 1;
        playerTeam[i]->y = playerType[i]->y + playerType[i]->width + 5;
        playerTeam[i]->alignment = CENTER;
        newGameMenu->addChild(*playerTeam[i]);

        playerTank[i] = TextBox::create();
        playerTank[i]->width = 9;
        playerTank[i]->height = 1;
        playerTank[i]->x = i * 2 + 1;
        playerTank[i]->y = playerTeam[i]->y + playerTeam[i]->width + 5;
        playerTank[i]->text = "Customize";
        newGameMenu->addChild(*playerTank[i]);
    }

    newGameStartText = TextBox::create();
    newGameStartText->width = 10;
    newGameStartText->height = 1;
    newGameStartText->x = playerText[3]->x + 3;
    newGameStartText->y = newGameMenu->width - newGameStartText->width;
    newGameStartText->text = "Start game";
    newGameStartText->color = WHITE;
    newGameMenu->addChild(*newGameStartText);

    std::shared_ptr<TextBox> gameOptions = TextBox::create();
    gameOptions->width = 12;
    gameOptions->height = 1;
    gameOptions->x = newGameStartText->x;
    gameOptions->y = newGameStartText->y - gameOptions->width - 4;
    gameOptions->text = "Game options";
    gameOptions->color = WHITE;
    newGameMenu->addChild(*gameOptions);

    std::shared_ptr<Form> newGameForm = std::make_shared<Form>(NUM_NEW_GAME_AREAS);
    for (int i = 0; i < 4; i++) {
        newGameForm->elements[i * NUM_PLAYER_NEW_GAME_AREAS + TANK_TYPE_OPTION] = playerType[i];
        newGameForm->elements[i * NUM_PLAYER_NEW_GAME_AREAS + TANK_TEAM_OPTION] = playerTeam[i];
        newGameForm->elements[i * NUM_PLAYER_NEW_GAME_AREAS + TANK_CUSTOMIZE_OPTION] = playerTank[i];

        newGameForm->actions[i * NUM_PLAYER_NEW_GAME_AREAS + TANK_TYPE_OPTION] = [](Form::event_args_t e)->bool {
            const int idx = e.position / NUM_PLAYER_NEW_GAME_AREAS;
            if (newGameSettings.players[idx].enabled) {
                if (newGameSettings.players[idx].human) {
                    newGameSettings.players[idx].human = false;
                    newGameSettings.players[idx].difficulty = BOT_EASY;
                } else {
                    newGameSettings.players[idx].difficulty = (BotDifficulty)
                        ((newGameSettings.players[idx].difficulty + 1) % NUM_BOT_DIFFICULTIES);
                    if (newGameSettings.players[idx].difficulty == BOT_EASY)
                        newGameSettings.players[idx].enabled = false;
                }
            } else {
                newGameSettings.players[idx].enabled = true;
                newGameSettings.players[idx].human = true;
            }

            e.form->isFocused = false;
            return true;
        };

        newGameForm->actions[i * NUM_PLAYER_NEW_GAME_AREAS + TANK_TEAM_OPTION] = [](Form::event_args_t e)->bool {
            const int idx = e.position / NUM_PLAYER_NEW_GAME_AREAS;
            if (newGameSettings.players[idx].enabled)
                newGameSettings.players[idx].team = newGameSettings.players[idx].team % 4 + 1;
            
            e.form->isFocused = false;
            return true;
        };

        newGameForm->actions[i * NUM_PLAYER_NEW_GAME_AREAS + TANK_CUSTOMIZE_OPTION] = [](Form::event_args_t e)->bool {
            const int idx = e.position / NUM_PLAYER_NEW_GAME_AREAS;
            if (newGameSettings.players[idx].enabled) {
                callWithConsoleSnapshot([idx]() {
                    customizeTankMenu(idx);
                });
            }

            e.form->isFocused = false;
            return true;
        };
    }
    newGameForm->elements[GAME_OPTIONS_LEFT] = gameOptions;
    newGameForm->elements[GAME_OPTIONS_RIGHT] = gameOptions;
    newGameForm->elements[START_GAME_OPTION] = newGameStartText;
    Form::configureMatrixForm(*newGameForm, 5, NUM_PLAYER_NEW_GAME_AREAS);
    newGameForm->mapping[GAME_OPTIONS_LEFT].right = START_GAME_OPTION;
    newGameForm->actions[GAME_OPTIONS_LEFT] = [](Form::event_args_t e)->bool {
        callWithConsoleSnapshot(gameOptionsMenu);

        e.form->isFocused = false;
        return true;
    };
    newGameForm->actions[GAME_OPTIONS_RIGHT] = newGameForm->actions[GAME_OPTIONS_LEFT];
    newGameForm->actions[START_GAME_OPTION] = startGameAction;

    tickLoop([&]() {
        newGameForm->tick(true, [&](Form::event_args_t e) {
            if (e.type == Form::KEY && e.record.wVirtualKeyCode == VK_ESCAPE)
                exitNewGame = true;
        });
    }, [&]()->bool {
        if (exitNewGame) {
            exitNewGame = false;
            return false;
        }

        console->clear(BLACK);

        updateNewGameMenu();

        newGameMenu->draw(*console);
        newGameForm->draw(*console, *newGameMenu);

        console->commit();
        return true;
    });
}

enum MainMenuArea {
    NEW_GAME_OPTION,
    LOAD_GAME_OPTION,
    WEAPON_TEST_OPTION,
    EXIT_GAME_OPTION,
    NUM_MAIN_MENU_AREAS
};

static bool newGameAction(Form::event_args_t e) {
    callWithConsoleSnapshot(newGameMenu);

    e.form->isFocused = false;
    return true;
}

static bool loadGameAction(Form::event_args_t e) {
    if (loadGame())
        runGameLoop();

    e.form->isFocused = false;
    return true;
}

static bool weaponTestAction(Form::event_args_t e) {
    match = std::make_shared<TankMatch>();
    match->buildMap([](float x) { return 0.5f; });

    std::shared_ptr<Tank> tank = Tank::create(RED);
    match->addEntity(*tank);

    std::shared_ptr<TankController> controller = TankController::create();
    controller->tank = tank;
    match->players.push_back(controller);

    for (const std::shared_ptr<Weapon> &weapon : TankMatch::weapons)
        controller->weapons.push_back(std::make_pair(weapon, TankMatch::UNLIMITED_WEAPON_THRESHOLD));

    match->arrangeTanks();

    runGameLoop();
    
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
    mainMenu->width = MENU_WIDTH / 2 - 12;
    mainMenu->height = MENU_HEIGHT - 12;
    mainMenu->x = 10;
    mainMenu->y = 6;

    std::shared_ptr<TextBox> newGameOption = TextBox::create();
    newGameOption->width = mainMenu->width - 2;
    newGameOption->height = 1;
    newGameOption->x = 1;
    newGameOption->y = 1;
    newGameOption->text = "> New Game";
    mainMenu->addChild(*newGameOption);

    std::shared_ptr<TextBox> loadGameOption = TextBox::create();
    loadGameOption->width = mainMenu->width - 2;
    loadGameOption->height = 1;
    loadGameOption->x = 4;
    loadGameOption->y = 1;
    loadGameOption->text = "> Load Game";
    mainMenu->addChild(*loadGameOption);

    std::shared_ptr<TextBox> weaponTestOption = TextBox::create();
    weaponTestOption->width = mainMenu->width - 2;
    weaponTestOption->height = 1;
    weaponTestOption->x = 7;
    weaponTestOption->y = 1;
    weaponTestOption->text = "> Weapon Test";
    mainMenu->addChild(*weaponTestOption);

    std::shared_ptr<TextBox> exitGameOption = TextBox::create();
    exitGameOption->width = mainMenu->width - 2;
    exitGameOption->height = 1;
    exitGameOption->x = 10;
    exitGameOption->y = 1;
    exitGameOption->text = "> Exit Game";
    mainMenu->addChild(*exitGameOption);

    std::shared_ptr<Form> mainMenuForm = std::make_shared<Form>(NUM_MAIN_MENU_AREAS);
    mainMenuForm->elements[NEW_GAME_OPTION] = newGameOption;
    mainMenuForm->elements[LOAD_GAME_OPTION] = loadGameOption;
    mainMenuForm->elements[WEAPON_TEST_OPTION] = weaponTestOption;
    mainMenuForm->elements[EXIT_GAME_OPTION] = exitGameOption;
    Form::configureSimpleForm(*mainMenuForm);
    mainMenuForm->actions[NEW_GAME_OPTION] = newGameAction;
    mainMenuForm->actions[LOAD_GAME_OPTION] = loadGameAction;
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

    initWindowsColors();
    TankMatch::initalizeWeapons();

    newGameSettings.players[0].enabled = true;
    newGameSettings.players[0].human = true;
    newGameSettings.players[1].enabled = true;
    for (int i = 0; i < 4; i++) {
        newGameSettings.players[i].team = i + 1;
        for (int j = 0; j < NUM_TANK_ATTRIBUTES; j++)
            newGameSettings.players[i].tank[j] = DEFAULT_TANK_ATTRIBUTE_VALUE;
    }

    AttachConsole(-1);
    preventResizeWindow();

    try {
        mainMenu();
    } catch (std::exception &e) {
        messageBox(e.what(), "Fatal error!");
    }
}
