#include "Game/TankWheel.h"


namespace Hilltop {
namespace Game {

bool TankWheel::enableDebug = false;

TankWheel::TankWheel() : Entity() {}

std::shared_ptr<TankWheel> TankWheel::create() {
    return std::shared_ptr<TankWheel>(new TankWheel());
}

void TankWheel::onDraw(TankMatch *match, Console::DoublePixelBufferedConsole &console) {
    Entity::onDraw(match, console);

    if (!enableDebug)
        return;

    Vector2 p = position.round();
    if (entityAge % 2 == 0)
        console.set(p.X, p.Y, Console::RED);
}

}
}
