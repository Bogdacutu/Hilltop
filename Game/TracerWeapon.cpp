#include "Game/TracerWeapon.h"
#include "Game/TankController.h"
#include "Game/TankMatch.h"
#include "Game/Tracer.h"
#include <sstream>


namespace Hilltop {
namespace Game {

TracerWeapon::TracerWeapon() : Weapon() {}

void TracerWeapon::fire(TankMatch &match, int playerNumber) {
    std::shared_ptr<Tank> tank = match.players[playerNumber]->tank;
    for (int i = -TRACER_OFFSET; i <= TRACER_OFFSET; i += TRACER_INTERVAL) {
        std::shared_ptr<Tracer> tracer = Tracer::create();
        tracer->position = tank->getProjectileBase();
        tracer->direction = Tank::calcTrajectory(tank->angle + i, tank->power);
        std::ostringstream text;
        text << i;
        tracer->text = text.str();
        match.addEntity(*tracer);
    }
}

}
}
