#include "Game/BouncyRocketWeapon.h"
#include "Game/BouncyTrailedRocket.h"


namespace Hilltop {
namespace Game {

std::shared_ptr<Entity> BouncyRocketWeapon::createRocket(Vector2 position,
    Vector2 direction) {
    std::shared_ptr<BouncyTrailedRocket> rocket = BouncyTrailedRocket::create();
    rocket->position = position;
    rocket->direction = direction;
    return rocket;
}

BouncyRocketWeapon::BouncyRocketWeapon() : RocketWeapon(1) {}

}
}
