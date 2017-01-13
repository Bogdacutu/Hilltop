#include "Game/BulletRainWeapon.h"
#include "Game/BulletRainCloud.h"
#include "Game/Tank.h"
#include "Game/TankController.h"
#include "Game/TankMatch.h"


namespace Hilltop {
namespace Game {

Hilltop::Game::BulletRainWeapon::BulletRainWeapon() : Weapon() {}

void Hilltop::Game::BulletRainWeapon::fire(TankMatch &match, int playerNumber) {
    std::shared_ptr<Tank> tank = match.players[playerNumber]->tank;
    std::shared_ptr<BulletRainCloud> cloud = BulletRainCloud::create();
    cloud->position = tank->getProjectileBase();
    cloud->direction = tank->calcTrajectory();
    match.addEntity(*cloud);
}

}
}
