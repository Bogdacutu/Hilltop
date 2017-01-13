#include "Game/DirtRocketWeapon.h"
#include "Game/SimpleRocket.h"


namespace Hilltop {
namespace Game {

std::shared_ptr<Entity> DirtRocketWeapon::createRocket(Vector2 position,
    Vector2 direction) {
    std::shared_ptr<SimpleRocket> rocket =
        std::dynamic_pointer_cast<SimpleRocket>(RocketWeapon::createRocket(position, direction));
    rocket->destroyLand = false;
    rocket->createLand = true;
    rocket->explosionSize = explosionSize;
    return rocket;
}

DirtRocketWeapon::DirtRocketWeapon(int numRockets) : RocketWeapon(numRockets) {}

}
}
