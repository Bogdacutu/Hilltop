#include "Game/SimpleTrailedRocket.h"
#include "Game/RocketTrail.h"
#include "Game/TankMatch.h"


namespace Hilltop {
namespace Game {

SimpleTrailedRocket::SimpleTrailedRocket(Console::ConsoleColor color, Console::ConsoleColor trailColor,
    int trailTime) : SimpleRocket(color), trailColor(trailColor), trailTime(trailTime) {}

std::shared_ptr<SimpleTrailedRocket> SimpleTrailedRocket::create(Console::ConsoleColor color,
    Console::ConsoleColor trailColor, int trailTime) {
    return std::shared_ptr<SimpleTrailedRocket>(new SimpleTrailedRocket(color, trailColor, trailTime));
}

void SimpleTrailedRocket::onTick(TankMatch *match) {
    SimpleRocket::onTick(match);

    if (!hasHit || groundHog) {
        Vector2 from = position - match->gravity * gravityMult;
        Vector2 to = from + direction;
        to = match->checkForHit(position, to, groundHog).second.round();
        foreachPixel(from, to, [this, match, to](Vector2 p)->bool {
            if (p.round() == to)
                return true;

            std::shared_ptr<RocketTrail> trail = RocketTrail::create(trailTime, trailColor);
            trail->position = p;
            match->addEntity(*trail);
            return false;
        });
    }
}

}
}
