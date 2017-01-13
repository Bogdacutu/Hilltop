#include "Game/Tracer.h"
#include "Console/Text.h"
#include "Game/TankMatch.h"


namespace Hilltop {
namespace Game {

Tracer::Tracer() {}

std::shared_ptr<Tracer> Tracer::create() {
    return std::shared_ptr<Tracer>(new Tracer());
}

void Tracer::onTick(TankMatch *match) {
    Entity::onTick(match);

    match->updateMattered = true;
}

void Tracer::onDraw(TankMatch *match, Console::DoublePixelBufferedConsole &console) {
    Entity::onDraw(match, console);

    Vector2 p = position.round();
    console.set(p.X, p.Y, Console::WHITE);
}

void Tracer::onDirectDraw(TankMatch *match, Console::BufferedConsole &console) {
    Entity::onDirectDraw(match, console);

    Vector2 p = position.round();
    int top = (p.X - 3) / 2;
    printText(&console, top, p.Y - text.length() / 2, (int)text.length(), 1, text, Console::WHITE,
        Console::LEFT, false);
}

void Tracer::onHit(TankMatch *match) {
    if (!hasHit) {
        maxEntityAge = entityAge + DURATION;
        gravityMult = 0.0f;
    }

    Entity::onHit(match);
}

}
}
