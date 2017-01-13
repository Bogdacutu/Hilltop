#include "Game/Minigun.h"
#include "Game/SimpleRocket.h"
#include "Game/TankMatch.h"


namespace Hilltop {
namespace Game {

Minigun::Minigun() : Entity() {
    position = Vector2(0, 0);
    direction = Vector2(0, 0);
    gravityMult = 0;
    maxEntityAge = MINIGUN_TICKS;
}

std::shared_ptr<Minigun> Minigun::create() {
    return std::shared_ptr<Minigun>(new Minigun());
}

void Minigun::onTick(TankMatch *match) {
    Entity::onTick(match);

    int offset = scale(rand(), 0, RAND_MAX, -ANGLE_OFFSET - 1, ANGLE_OFFSET);
    std::shared_ptr<SimpleRocket> rocket = SimpleRocket::create(Console::YELLOW);
    rocket->explosionSize = 3;
    rocket->position = tank->getBarrelBase() + Tank::getProjectileBase(tank->angle + offset);
    rocket->direction = Tank::calcTrajectory(tank->angle + offset, tank->power);
    rocket->explosionDamage = 0.3f;
    match->addEntity(*rocket);
}

}
}
