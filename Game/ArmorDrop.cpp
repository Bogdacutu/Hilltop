#include "Game/ArmorDrop.h"


namespace Hilltop {
namespace Game {

ArmorDrop::ArmorDrop() : Drop() {
    color = Console::BLUE;
    ch = L'A';
}

std::shared_ptr<ArmorDrop> ArmorDrop::create() {
    return std::shared_ptr<ArmorDrop>(new ArmorDrop());
}

void ArmorDrop::handleTank(TankMatch *match, Tank &tank) {
    tank.armor = std::min(tank.maxArmor, tank.armor + ARMOR);
}

}
}
