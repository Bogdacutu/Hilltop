#include "Game/WeaponDrop.h"
#include "Game/TankController.h"


namespace Hilltop {
namespace Game {

WeaponDrop::WeaponDrop() : Drop() {
    color = Console::GREEN;
    ch = L'W';
}

std::shared_ptr<WeaponDrop> WeaponDrop::create() {
    return std::shared_ptr<WeaponDrop>(new WeaponDrop());
}

void WeaponDrop::handleTank(TankMatch *match, Tank &tank) {
    for (const std::shared_ptr<TankController> &player : match->players) {
        if (player->tank.get() == &tank) {
            for (int i = 0; i < WEAPONS; i++)
                player->addRandomWeapon();
            break;
        }
    }
}

}
}
