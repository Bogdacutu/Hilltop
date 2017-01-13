#include "Game/ParticleBomb.h"
#include "Game/TankController.h"
#include "Game/TankMatch.h"


namespace Hilltop {
namespace Game {

ParticleBomb::ParticleBomb() : SimpleTrailedRocket(Console::YELLOW, Console::DARK_GRAY, 1) {
    explosionSize = 1;
    destroyLand = false;
}

std::shared_ptr<ParticleBomb> ParticleBomb::create() {
    return std::shared_ptr<ParticleBomb>(new ParticleBomb());
}

void ParticleBomb::onTick(TankMatch *match) {
    SimpleTrailedRocket::onTick(match);

    for (int i = 0; i < match->players.size(); i++) {
        if (match->players[i]->team != team && match->players[i]->tank->alive) {
            std::shared_ptr<Tank> tank = match->players[i]->tank;
            if (std::abs(position.X - tank->getBarrelBase().X) < TRIGGER_DISTANCE_X &&
                std::abs(position.Y - tank->getBarrelBase().Y) < TRIGGER_DISTANCE_Y) {
                for (int i = 0; i < STEPS; i++) {
                    int angle = 360 / STEPS * i;
                    std::shared_ptr<SimpleTrailedRocket> rocket =
                        SimpleTrailedRocket::create(Console::WHITE, Console::DARK_GRAY, 3);
                    rocket->position = position.round();
                    rocket->direction = Tank::calcTrajectory(angle, 10);
                    rocket->explosionSize = 3;
                    rocket->explosionDamage = 0.6f;
                    match->addEntity(*rocket);
                }
                onHit(match);
                break;
            }
        }
    }
}

}
}
