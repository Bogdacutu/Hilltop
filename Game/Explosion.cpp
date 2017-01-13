#include "Game/Explosion.h"
#include "Game/TankController.h"
#include "Game/TankMatch.h"
#include "resource.h"
#include <Windows.h>


namespace Hilltop {
namespace Game {

static void playSound() {
    PlaySound(MAKEINTRESOURCE(IDR_WAVE1), GetModuleHandle(nullptr), SND_RESOURCE | SND_ASYNC);
}

Explosion::Explosion(int size) : Entity(), size(size) {
    gravityMult = 0.0f;
    coreSize = size - 1;

    playSound();
}

void Explosion::destroyLand(TankMatch *match) {
    Vector2 p = position.round();
    for (int i = p.X - size; i <= p.X + size; i++)
        for (int j = p.Y - size; j <= p.Y + size; j++)
            if (distance(Vector2(i, j), p) < size)
                match->set(i, j, TankMatch::AIR);
}

void Explosion::createLand(TankMatch *match) {
    Vector2 p = position.round();
    for (int i = p.X - size; i <= p.X + size; i++)
        for (int j = p.Y - size; j <= p.Y + size; j++)
            if (distance(Vector2(i, j), p) < size)
                if (match->get(i, j) == TankMatch::AIR)
                    match->set(i, j, TankMatch::DIRT);
}

int Explosion::calcDamage(Vector2 point) {
    return std::max<int>(1, scale(distance(position.round(), point), 0, size, 8 * size, 1) * damageMult);
}

void Explosion::hitTanks(TankMatch *match) {
    Vector2 p = position.round();
    typedef std::pair<std::shared_ptr<Tank>, Vector2> hit_t;
    std::set<hit_t, std::function<bool(hit_t, hit_t)>> hits([p](hit_t x, hit_t y)->bool {
        return distance(p, x.second) < distance(p, y.second);
    });

    for (int i = 0; i < match->players.size(); i++)
        for (const Vector2 &v : match->players[i]->tank->getPixels())
            if (distance(p, v) < size)
                hits.insert(std::make_pair(match->players[i]->tank, v));

    for (const hit_t &v : hits) {
        if (tanksHit.find(v.first) == tanksHit.end()) {
            tanksHit.insert(v.first);
            v.first->dealDamage(match, calcDamage(v.second));
        }
    }
}

std::shared_ptr<Explosion> Explosion::create(int size) {
    return std::shared_ptr<Explosion>(new Explosion(size));
}

void Explosion::onTick(TankMatch *match) {
    Entity::onTick(match);

    if (entityAge >= 2) {
        if (willDestroyLand) {
            willDestroyLand = false;
            destroyLand(match);
            hitTanks(match);
        }
        if (willCreateLand) {
            willCreateLand = false;
            createLand(match);
        }
    }

    if (entityAge % ticksBetween == 0) {
        if (coreSize <= 1) {
            match->removeEntity(*this);
        } else {
            coreSize /= 2;
        }
    }
}

void Explosion::onDraw(TankMatch *match, Console::DoublePixelBufferedConsole &console) {
    Entity::onDraw(match, console);

    Vector2 p = position.round();
    for (int i = p.X - size; i <= p.X + size; i++) {
        for (int j = p.Y - size; j <= p.Y + size; j++) {
            float dist = distance(Vector2(i, j), p) / (float)size;
            if (dist <= 1) {
                float core = (float)coreSize / (float)size;
                float outer = (float)(coreSize + 2) / (float)size;
                Console::ConsoleColor color = Console::DARK_GRAY;
                if (dist <= core) {
                    color = Console::BROWN;
                    if (dist <= 0.5)
                        color = Console::WHITE;
                    else if (dist <= 0.75)
                        color = Console::YELLOW;
                    else if (dist <= 0.85)
                        color = Console::ORANGE;
                }
                if (dist < outer)
                    console.set(i, j, color);
            }
        }
    }
}

}
}