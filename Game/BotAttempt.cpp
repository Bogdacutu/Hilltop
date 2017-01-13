#include "Game/BotAttempt.h"


namespace Hilltop {
namespace Game {

bool BotAttempt::enableDebug = false;

BotAttempt::BotAttempt() {}

std::shared_ptr<BotAttempt> BotAttempt::create() {
    return std::shared_ptr<BotAttempt>(new BotAttempt());
}

void BotAttempt::onDraw(TankMatch *match, Console::DoublePixelBufferedConsole &console) {
    Entity::onDraw(match, console);

    if (!enableDebug)
        return;

    Vector2 p = position.round();
    console.set(p.X, p.Y, color);
    if (color == Console::GREEN) {
        console.set(p.X - 1, p.Y, color);
        console.set(p.X + 1, p.Y, color);
        console.set(p.X, p.Y - 1, color);
        console.set(p.X, p.Y + 1, color);
    }
}

}
}
