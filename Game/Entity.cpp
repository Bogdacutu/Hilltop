#include "Game/Entity.h"
#include "Game/TankMatch.h"


namespace Hilltop {
namespace Game {

Entity::Entity() {}

Entity::~Entity() {}

std::shared_ptr<Entity> Entity::create() {
    return std::shared_ptr<Entity>(new Entity());
}

void Entity::onTick(TankMatch *match) {
    entityAge++;

    if (maxEntityAge >= 0)
        if (entityAge > maxEntityAge)
            onExpire(match);
}

void Entity::onDraw(TankMatch *match, Console::DoublePixelBufferedConsole &console) {}

void Entity::onDirectDraw(TankMatch *match, Console::BufferedConsole &console) {}

void Entity::onHit(TankMatch *match) {
    hasHit = true;
    direction = { 0.0f, 0.0f };
}

void Entity::onExpire(TankMatch *match) {
    hasExpired = true;
    match->removeEntity(*this);
}

}
}
