#include "Game/SimpleRocket.h"
#include "Game/Explosion.h"
#include "Game/TankMatch.h"


namespace Hilltop {
namespace Game {

SimpleRocket::SimpleRocket(Console::ConsoleColor color) : Entity(), color(color) {}

std::shared_ptr<SimpleRocket> SimpleRocket::create(Console::ConsoleColor color) {
    return std::shared_ptr<SimpleRocket>(new SimpleRocket(color));
}

void SimpleRocket::onDraw(TankMatch *match, Console::DoublePixelBufferedConsole &console) {
    Entity::onDraw(match, console);

    Vector2 p = position.round();
    console.set(p.X, p.Y, color);
}

void SimpleRocket::onHit(TankMatch *match) {
    Entity::onHit(match);

    match->removeEntity(*this);

    std::shared_ptr<Explosion> ex = Explosion::create(explosionSize);
    ex->position = position;
    ex->willDestroyLand = destroyLand;
    ex->willCreateLand = createLand;
    ex->damageMult = explosionDamage;
    match->addEntity(*ex);
}

}
}
