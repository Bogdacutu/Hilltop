#include "Game/RocketTrail.h"


namespace Hilltop {
namespace Game {

RocketTrail::RocketTrail(int maxAge, Console::ConsoleColor color) : Entity(), color(color) {
    maxEntityAge = maxAge;
    gravityMult = 0.0f;
}

std::shared_ptr<RocketTrail> RocketTrail::create(int maxAge, Console::ConsoleColor color) {
    return std::shared_ptr<RocketTrail>(new RocketTrail(maxAge, color));
}

void RocketTrail::onDraw(TankMatch *match, Console::DoublePixelBufferedConsole &console) {
    Entity::onDraw(match, console);

    Vector2 p = position.round();
    console.set(p.X, p.Y, color);
}

}
}
