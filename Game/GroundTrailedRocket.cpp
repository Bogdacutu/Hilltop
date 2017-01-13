#include "Game/GroundTrailedRocket.h"
#include "Game/TankController.h"
#include "Game/TankMatch.h"


namespace Hilltop {
namespace Game {

GroundTrailedRocket::GroundTrailedRocket() : SimpleTrailedRocket(Console::WHITE,
    Console::DARK_GRAY, 3) {}

std::shared_ptr<GroundTrailedRocket> GroundTrailedRocket::create() {
    return std::shared_ptr<GroundTrailedRocket>(new GroundTrailedRocket());
}

void GroundTrailedRocket::onTick(TankMatch *match) {
    SimpleTrailedRocket::onTick(match);

    if (entityAge <= 10)
        return;

    Vector2 p = position.round();
    for (int i = 0; i < match->players.size(); i++) {
        if (match->players[i]->tank->alive) {
            std::shared_ptr<Tank> tank = match->players[i]->tank;
            if (distance(p, tank->getBarrelBase()) <= 5.0f) {
                SimpleTrailedRocket::onHit(match);
                break;
            }
        }
    }
}

void GroundTrailedRocket::onHit(TankMatch *match) {
    if (!hasHit) {
        hasHit = true;
        gravityMult = -1.0f;
        groundHog = true;
    } else {
        Vector2 p = position.round();
        if (p.Y >= 0 && p.Y < match->width)
            SimpleTrailedRocket::onHit(match);
    }
}

}
}
