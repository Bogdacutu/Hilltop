#include "Game/GroundRocketWeapon.h"
#include "Game/GroundTrailedRocket.h"


namespace Hilltop {
namespace Game {

std::shared_ptr<Entity> GroundRocketWeapon::createRocket(Vector2 position,
    Vector2 direction) {
    std::shared_ptr<GroundTrailedRocket> rocket = GroundTrailedRocket::create();
    rocket->position = position;
    rocket->direction = direction;
    return rocket;
}

GroundRocketWeapon::GroundRocketWeapon(int numRockets) : RocketWeapon(numRockets) {}

}
}
