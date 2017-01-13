#include "Game/BulletRainCloud.h"
#include "Game/SimpleTrailedRocket.h"
#include "Game/TankMatch.h"


namespace Hilltop {
namespace Game {

BulletRainCloud::BulletRainCloud() : Entity() {}

std::shared_ptr<BulletRainCloud> BulletRainCloud::create() {
    return std::shared_ptr<BulletRainCloud>(new BulletRainCloud());
}

void BulletRainCloud::onTick(TankMatch *match) {
    Entity::onTick(match);

    if (hasHit && entityAge % BULLET_EVERY_TICKS == 0) {
        Vector2 p = position.round();
        int left = p.Y - CLOUD_WIDTH / 2;
        int right = p.Y + CLOUD_WIDTH / 2;
        int pos = scale(rand(), 0, RAND_MAX, left, right);
        std::shared_ptr<SimpleTrailedRocket> rocket =
            SimpleTrailedRocket::create(Console::YELLOW, Console::DARK_GRAY, 1);
        rocket->position = Vector2(-10.0f, (float)pos);
        rocket->explosionSize = 3;
        rocket->explosionDamage = RAIN_DAMAGE;
        match->addEntity(*rocket);
    }
}

void BulletRainCloud::onDraw(TankMatch *match, Console::DoublePixelBufferedConsole &console) {
    Entity::onDraw(match, console);

    Vector2 p = position.round();
    if (!hasHit)
        console.set(p.X, p.Y, Console::YELLOW);
}

void BulletRainCloud::onHit(TankMatch *match) {
    if (!hasHit)
        maxEntityAge = entityAge + RAIN_TICKS;

    Entity::onHit(match);
}

}
}
