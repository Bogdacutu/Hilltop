#include "Game/ParticleBombWeapon.h"
#include "Game/ParticleBomb.h"
#include "Game/TankController.h"
#include "Game/TankMatch.h"


namespace Hilltop {
namespace Game {

ParticleBombWeapon::ParticleBombWeapon() : Weapon() {}

void ParticleBombWeapon::fire(TankMatch &match, int playerNumber) {
    std::shared_ptr<Tank> tank = match.players[playerNumber]->tank;
    std::shared_ptr<ParticleBomb> bomb = ParticleBomb::create();
    bomb->position = tank->getProjectileBase();
    bomb->direction = tank->calcTrajectory();
    bomb->team = match.players[playerNumber]->team;
    match.addEntity(*bomb);
}

}
}
