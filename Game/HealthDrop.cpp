#include "Game/HealthDrop.h"


namespace Hilltop {
namespace Game {

HealthDrop::HealthDrop() : Drop() {
    color = Console::RED;
    ch = L'H';
}

std::shared_ptr<HealthDrop> HealthDrop::create() {
    return std::shared_ptr<HealthDrop>(new HealthDrop());
}

void HealthDrop::handleTank(TankMatch *match, Tank &tank) {
    tank.health = std::min(tank.maxHealth, tank.health + HEALTH);
}

}
}
