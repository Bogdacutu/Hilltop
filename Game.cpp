#include "Game.h"
#include "Console.h"

using namespace Hilltop::Console;
using namespace Hilltop::Game;

Hilltop::Game::Projectile::~Projectile() {}

static const ConsoleColor LAND_COLORS[NUM_LAND_TYPES] = { DARK_BLUE, DARK_GREEN, BROWN };

LandType Hilltop::Game::TankMatch::get(unsigned short x, unsigned short y) {
    if (x >= height || y >= width)
        return AIR;

    return map[x * width + y];
}

void Hilltop::Game::TankMatch::set(unsigned short x, unsigned short y, LandType type) {
    if (x >= height || y >= width)
        return;

    map[x * width + y] = type;
}

Hilltop::Game::TankMatch::TankMatch(unsigned short width, unsigned short height)
    : width(width), height(height), map(width * height) {}

void Hilltop::Game::TankMatch::buildMap() {
    for (int i = height / 2; i < height; i++) {
        for (int j = 0; j < width; j++) {
            set(i, j, GRASS);
        }
    }
}

void Hilltop::Game::TankMatch::draw(Console::DoublePixelBufferedConsole &console) {
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            console.set(i, j, LAND_COLORS[get(i, j)]);
        }
    }
}

void Hilltop::Game::TankMatch::doTick(uint64_t tickNumber) {

}
