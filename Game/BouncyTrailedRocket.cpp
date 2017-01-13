#include "Game/BouncyTrailedRocket.h"
#include "Game/TankController.h"
#include "Game/TankMatch.h"


namespace Hilltop {
namespace Game {

BouncyTrailedRocket::BouncyTrailedRocket() : SimpleTrailedRocket(Console::BLUE,
    Console::DARK_GRAY, 2) {}

std::shared_ptr<BouncyTrailedRocket> BouncyTrailedRocket::create() {
    return std::shared_ptr<BouncyTrailedRocket>(new BouncyTrailedRocket());
}

void BouncyTrailedRocket::onHit(TankMatch *match) {
    Vector2 d = direction;

    Entity::onHit(match);

    Vector2 p = position.round();
    Vector2 air;
    bool foundAir = false;
    for (int i = -1; i <= 1; i++) {
        for (int j = -1; j <= 1; j++) {
            if (match->get(p.X + i, p.Y + j) == TankMatch::AIR) {
                Vector2 a = Vector2(i, j);
                if (!foundAir) {
                    foundAir = true;
                    air = a;
                } else if (distance(p, p + air) > distance(p, p + a)) {
                    air = a;
                    break;
                }
            }
        }
    }

    bool foundTank = false;
    for (int i = 0; i < match->players.size(); i++) {
        if (match->players[i]->tank->alive) {
            std::shared_ptr<Tank> tank = match->players[i]->tank;
            if (distance(p, tank->getBarrelBase()) <= 6.0f) {
                foundTank = true;
                break;
            }
        }
    }

    bouncesLeft--;
    if (bouncesLeft <= 0 || !foundAir || foundTank) {
        finish(match);
    } else {
        hasHit = false;
        if ((air.X < 0.0f && d.X > 0.0f) || (air.X > 0.0f && d.X < 0.0f))
            d.X *= -1.0f;
        if ((air.Y < 0.0f && d.Y > 0.0f) || (air.Y > 0.0f && d.Y < 0.0f))
            d.Y *= -1.0f;
        direction = d;
        position = position + air;
    }
}

void BouncyTrailedRocket::finish(TankMatch *match) {
    SimpleTrailedRocket::onHit(match);
}

}
}
