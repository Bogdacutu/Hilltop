#include "Game/Drop.h"
#include "Game/TankController.h"


namespace Hilltop {
namespace Game {

Drop::Drop() : Entity() {}

int Drop::getTopRow() {
    Vector2 p = position.round();
    return ((p.X - 5) / 2) * 2;
}

void Drop::onTick(TankMatch *match) {
    Entity::onTick(match);

    int top = getTopRow();
    int left = position.round().Y;

    for (int i = 0; i < 6; i++) {
        bool found = false;
        for (const std::shared_ptr<TankController> &player : match->players) {
            if (player->tank->alive && player->tank->testCollision(Vector2(top + i, left))) {
                handleTank(match, *player->tank);
                match->removeEntity(*this);
                found = true;
                break;
            }
        }
        if (found)
            break;
    }
}

void Drop::onDirectDraw(TankMatch *match, Console::BufferedConsole &console) {
    Entity::onDirectDraw(match, console);

    int top = getTopRow() / 2;
    int left = position.round().Y;

    console.set(top, left, L'▄', make_color(color, Console::WHITE));
    console.set(top + 1, left, ch, make_color(Console::WHITE, color));
    console.set(top + 2, left, L'▄', make_color(Console::WHITE, color));
}

void Drop::handleTank(TankMatch *match, Tank &tank) {}

}
}
