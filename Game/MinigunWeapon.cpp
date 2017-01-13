#include "Game/MinigunWeapon.h"
#include "Game/Minigun.h"
#include "Game/TankController.h"
#include "Game/TankMatch.h"


namespace Hilltop {
namespace Game {

MinigunWeapon::MinigunWeapon() : Weapon() {}

void MinigunWeapon::fire(TankMatch &match, int playerNumber) {
    std::shared_ptr<Minigun> minigun = Minigun::create();
    minigun->tank = match.players[playerNumber]->tank;
    match.addEntity(*minigun);
}

}
}
