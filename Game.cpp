#include "Game.h"
#include "Console.h"
#include <algorithm>

using namespace Hilltop::Console;
using namespace Hilltop::Game;




float Hilltop::Game::scale(float value, float fromLow, float fromHigh, float toLow, float toHigh) {
    return (value - fromLow) * (toHigh - toLow) / (fromHigh - fromLow) + toLow;
}

float Hilltop::Game::distance(const Vector2 from, const Vector2 to) {
    Vector2 v = to - from;
    return std::sqrt(v.X * v.X + v.Y * v.Y);
}

void Hilltop::Game::foreachPixel(const Vector2 from, const Vector2 to, std::function<bool(Vector2)> handler) {
    Vector2 start = from.round();
    Vector2 end = to.round();
    int steps = std::ceil(std::max(std::abs(start.X - end.X), std::abs(start.Y - end.Y)));
    Vector2 lastPixel = start;
    for (int i = 0; i <= steps; i++) {
        float pos = scale(i, 0, steps, 0, 1);
        Vector2 p = (start + (end - start) * pos).round();
        if (i == 0 || p != lastPixel) {
            lastPixel = p;
            if (handler(p))
                break;
        }
    }
}



//
// Projectile
//

Hilltop::Game::Projectile::Projectile() {}

Hilltop::Game::Projectile::~Projectile() {}

void Hilltop::Game::Projectile::onTick(TankMatch *match) {}

void Hilltop::Game::Projectile::onDraw(TankMatch *match, Console::DoublePixelBufferedConsole &console) {}

void Hilltop::Game::Projectile::onHit(TankMatch *match) {
    hasHit = true;
}



//
// SimpleMissle
//

Hilltop::Game::SimpleMissle::SimpleMissle(ConsoleColor color) : Projectile(), color(color) {}

std::shared_ptr<SimpleMissle> Hilltop::Game::SimpleMissle::create(ConsoleColor color) {
    return std::shared_ptr<SimpleMissle>(new SimpleMissle(color));
}

void Hilltop::Game::SimpleMissle::onDraw(TankMatch *match, Console::DoublePixelBufferedConsole &console) {
    Projectile::onDraw(match, console);

    Vector2 p = position.round();
    console.set(p.X, p.Y, WHITE);
}

void Hilltop::Game::SimpleMissle::onHit(TankMatch *match) {
    Projectile::onHit(match);

    match->removeProjectile(*this);

    std::shared_ptr<Explosion> ex = Explosion::create(4, 4);
    ex->position = position;
    ex->willDestroyLand = true;
    match->addProjectile(*ex);
}



//
// SimpleTrailedMissle
//

Hilltop::Game::SimpleTrailedMissile::SimpleTrailedMissile(ConsoleColor color, ConsoleColor trailColor,
    int trailTime) : SimpleMissle(color), trailColor(trailColor), trailTime(trailTime) {}

std::shared_ptr<SimpleTrailedMissile> Hilltop::Game::SimpleTrailedMissile::create(ConsoleColor color, ConsoleColor trailColor, int trailTime) {
    return std::shared_ptr<SimpleTrailedMissile>(new SimpleTrailedMissile(color, trailColor, trailTime));
}

void Hilltop::Game::SimpleTrailedMissile::onTick(TankMatch *match) {
    SimpleMissle::onTick(match);

    if (!hasHit) {
        Vector2 from = position - gravity;
        Vector2 to = from + direction;
        to = match->checkForHit(position, to).second.round();
        foreachPixel(from, to, [this, match, to](Vector2 p)->bool {
            if (p.round() == to)
                return true;

            std::shared_ptr<MissleTrail> trail = MissleTrail::create(trailTime, trailColor);
            trail->position = p;
            match->addProjectile(*trail);
            return false;
        });
    }
}



//
// MissleTrail
//

Hilltop::Game::MissleTrail::MissleTrail(int maxAge, ConsoleColor color)
    : Projectile(), maxAge(maxAge), color(color) {
    gravity = { 0, 0 };
}

std::shared_ptr<MissleTrail> Hilltop::Game::MissleTrail::create(int maxAge, ConsoleColor color) {
    return std::shared_ptr<MissleTrail>(new MissleTrail(maxAge, color));
}

void Hilltop::Game::MissleTrail::onTick(TankMatch *match) {
    Projectile::onTick(match);

    if (age >= maxAge)
        match->removeProjectile(*this);
    else
        age++;
}

void Hilltop::Game::MissleTrail::onDraw(TankMatch *match, Console::DoublePixelBufferedConsole &console) {
    Projectile::onDraw(match, console);

    console.set(position.X, position.Y, color);
}



//
// Explosion
//

Hilltop::Game::Explosion::Explosion(int size, int damage) : Projectile(), size(size), damage(damage) {
    gravity = { 0, 0 };
    coreSize = size - 1;
}

std::shared_ptr<Explosion> Hilltop::Game::Explosion::create(int size, int damage) {
    return std::shared_ptr<Explosion>(new Explosion(size, damage));
}

void Hilltop::Game::Explosion::onTick(TankMatch *match) {
    Projectile::onTick(match);

    if (!firstTick) {
        if (willDestroyLand) {
            willDestroyLand = false;
            destroyLand(match);
        }
    }

    ticksLeft--;
    if (ticksLeft == 0) {
        ticksLeft = ticksBetween;

        if (coreSize <= 1) {
            match->removeProjectile(*this);
        } else {
            coreSize /= 2;
        }
    }
}

void Hilltop::Game::Explosion::onDraw(TankMatch *match, Console::DoublePixelBufferedConsole &console) {
    Projectile::onDraw(match, console);

    Vector2 p = position.round();
    for (int i = p.X - size; i <= p.X + size; i++) {
        for (int j = p.Y - size; j <= p.Y + size; j++) {
            float dist = distance(Vector2(i, j), p) / (float)size;
            if (dist <= 1) {
                float core = (float)coreSize / (float)size;
                float outer = (float)(coreSize + 2) / (float)size;
                ConsoleColor color = DARK_GRAY;
                if (dist <= core) {
                    color = BROWN;
                    if (dist <= 0.5)
                        color = WHITE;
                    else if (dist <= 0.75)
                        color = YELLOW;
                    else if (dist <= 0.85)
                        color = ORANGE;
                }
                if (dist < outer)
                    console.set(i, j, color);
            }
        }
    }

    firstTick = false;
}

void Hilltop::Game::Explosion::destroyLand(TankMatch *match) {
    Vector2 p = position.round();
    for (int i = p.X - size; i <= p.X + size; i++)
        for (int j = p.Y - size; j <= p.Y + size; j++)
            if (distance(Vector2(i, j), p) < size)
                match->set(i, j, AIR);
}



//
// TankMatch
//

static const ConsoleColor LAND_COLORS[NUM_LAND_TYPES] = { DARK_BLUE, DARK_GREEN, BROWN, BLACK };

void Hilltop::Game::TankMatch::doProjectileTick() {
    for (const std::shared_ptr<Projectile> &p : projectiles) {
        p->onTick(this);
    }

    while (!projectileChanges.empty()) {
        std::pair<bool, std::shared_ptr<Projectile>> ev = projectileChanges.front();
        projectileChanges.pop();

        if (ev.first) {
            projectiles.push_back(ev.second);
        } else {
            std::vector<std::shared_ptr<Projectile>>::iterator it =
                std::find(projectiles.begin(), projectiles.end(), ev.second);
            if (it != projectiles.end())
                projectiles.erase(it);
        }
    }

    for (const std::shared_ptr<Projectile> &p : projectiles) {
        Vector2 oldPos = p->position;
        Vector2 newPos = oldPos + p->direction;
        std::pair<bool, Vector2> hit = checkForHit(oldPos, newPos, p->groundHog);
        p->position = hit.second;
        if (hit.first)
            p->onHit(this);

        p->direction = p->direction + p->gravity;

        if (p->position.Y < 0 || p->position.Y >= width)
            removeProjectile(*p);
    }
}

Vector2 Hilltop::Game::TankMatch::calcTrajectory(int angle, int power) {
    const static float pi = std::atanf(1) * 4;
    float ang = (float)angle * pi / 180;
    return Vector2(-std::sinf(ang), std::cosf(ang)) * 8 * ((float)power / 100);
}

LandType Hilltop::Game::TankMatch::get(int x, int y) {
    if (x < 0 || x >= height || y < 0 || y >= width) {
        if (x == height)
            return DIRT;
        else
            return AIR;
    }

    return map[x * width + y];
}

void Hilltop::Game::TankMatch::set(int x, int y, LandType type) {
    if (x < 0 || x >= height || y < 0 || y >= width)
        return;

    map[x * width + y] = type;
}

Hilltop::Game::TankMatch::TankMatch(unsigned short width, unsigned short height)
    : width(width), height(height), map(width * height) {}

void Hilltop::Game::TankMatch::addProjectile(Projectile &projectile) {
    projectileChanges.push(make_pair(true, projectile.shared_from_this()));
}

void Hilltop::Game::TankMatch::removeProjectile(Projectile &projectile) {
    projectileChanges.push(make_pair(false, projectile.shared_from_this()));
}

void Hilltop::Game::TankMatch::buildMap(std::function<float(float)> generator) {
    for (int i = 0; i < width; i++) {
        int val = height - generator((float)i / width) * height;
        for (int j = height - 1; j >= val; j--)
            set(j, i, GRASS);
    }
}

std::pair<bool, Vector2> Hilltop::Game::TankMatch::checkForHit(const Vector2 from, const Vector2 to, bool groundHog) {
    std::pair<bool, Vector2> ret = std::make_pair(false, to);
    foreachPixel(from, to, [this, &ret, groundHog](Vector2 p)->bool {
        LandType land = get(p.X, p.Y);
        if ((land == AIR) == groundHog) {
            ret = std::make_pair(true, p);
            return true;
        }
        return false;
    });
    return ret;
}

void Hilltop::Game::TankMatch::draw(Console::DoublePixelBufferedConsole &console) {
    for (int i = 0; i < height; i++)
        for (int j = 0; j < width; j++)
            console.set(i, j, LAND_COLORS[get(i, j)]);

    for (const std::shared_ptr<Projectile> &p : projectiles)
        p->onDraw(this, console);
}

void Hilltop::Game::TankMatch::doTick(uint64_t tickNumber) {
    if (timeSinceLast == timeBetweenMissles) {
        timeSinceLast = 0;

        std::shared_ptr<Projectile> missle = SimpleTrailedMissile::create(YELLOW, DARK_GRAY, 3);
        missle->position = { 20, (float)width / 2 };
        int angle = scale(rand(), 0, RAND_MAX, 0, 359);
        int power = scale(rand(), 0, RAND_MAX, 20, 80);
        missle->direction = calcTrajectory(angle, power);
        addProjectile(*missle);
    }
    timeSinceLast++;

    doProjectileTick();
}
