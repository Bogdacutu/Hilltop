#include "Game/RocketWeapon.h"
#include "Game/SimpleTrailedRocket.h"
#include "Game/TankController.h"
#include "Game/TankMatch.h"


namespace Hilltop {
namespace Game {

std::shared_ptr<Entity> RocketWeapon::createRocket(Vector2 position,
    Vector2 direction) {
    std::shared_ptr<SimpleRocket> rocket =
        SimpleTrailedRocket::create(Console::WHITE, Console::DARK_GRAY, 3);
    rocket->position = position;
    rocket->direction = direction;
    rocket->explosionSize = explosionSize;
    return rocket;
}

RocketWeapon::RocketWeapon(int numRockets) : Weapon(), numRockets(numRockets) {}

void RocketWeapon::fire(TankMatch &match, int playerNumber) {
    Weapon::fire(match, playerNumber);

    std::shared_ptr<Tank> tank = match.players[playerNumber]->tank;
    int start = tank->angle - ((numRockets - 1) * 2) / 2;

    for (int i = 0; i < numRockets; i++) {
        Vector2 direction = tank->calcTrajectory(start + i * 2, tank->power);
        std::shared_ptr<Entity> rocket = createRocket(tank->getProjectileBase(), direction);
        match.addEntity(*rocket);
    }
}

}
}
