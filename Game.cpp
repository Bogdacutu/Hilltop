#include "Game.h"
#include "Console.h"
#include <algorithm>

#pragma warning(disable: 4244)

using namespace Hilltop::Console;
using namespace Hilltop::Game;


const static float PI = std::atanf(1) * 4;


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
    int steps = std::max(1, (int)std::ceil(std::max(std::abs(start.X - end.X), std::abs(start.Y - end.Y))));
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
// Entity
//

Hilltop::Game::Entity::Entity() {}

Hilltop::Game::Entity::~Entity() {}

std::shared_ptr<Entity> Hilltop::Game::Entity::create() {
    return std::shared_ptr<Entity>(new Entity());
}

void Hilltop::Game::Entity::onTick(TankMatch *match) {
    entityAge++;

    if (maxEntityAge >= 0)
        if (entityAge > maxEntityAge)
            onExpire(match);
}

void Hilltop::Game::Entity::onDraw(TankMatch *match, Console::DoublePixelBufferedConsole &console) {}

void Hilltop::Game::Entity::onHit(TankMatch *match) {
    hasHit = true;
    direction = { 0.0f, 0.0f };
}

void Hilltop::Game::Entity::onExpire(TankMatch *match) {
    match->removeEntity(*this);
}



//
// SimpleRocket
//

Hilltop::Game::SimpleRocket::SimpleRocket(ConsoleColor color) : Entity(), color(color) {}

std::shared_ptr<SimpleRocket> Hilltop::Game::SimpleRocket::create(ConsoleColor color) {
    return std::shared_ptr<SimpleRocket>(new SimpleRocket(color));
}

void Hilltop::Game::SimpleRocket::onDraw(TankMatch *match, Console::DoublePixelBufferedConsole &console) {
    Entity::onDraw(match, console);

    Vector2 p = position.round();
    console.set(p.X, p.Y, color);
}

void Hilltop::Game::SimpleRocket::onHit(TankMatch *match) {
    Entity::onHit(match);

    match->removeEntity(*this);

    std::shared_ptr<Explosion> ex = Explosion::create(8, 4);
    ex->position = position;
    ex->willDestroyLand = destroyLand;
    ex->willCreateLand = createLand;
    match->addEntity(*ex);
}



//
// SimpleTrailedRocket
//

Hilltop::Game::SimpleTrailedRocket::SimpleTrailedRocket(ConsoleColor color, ConsoleColor trailColor,
    int trailTime) : SimpleRocket(color), trailColor(trailColor), trailTime(trailTime) {}

std::shared_ptr<SimpleTrailedRocket> Hilltop::Game::SimpleTrailedRocket::create(ConsoleColor color, ConsoleColor trailColor, int trailTime) {
    return std::shared_ptr<SimpleTrailedRocket>(new SimpleTrailedRocket(color, trailColor, trailTime));
}

void Hilltop::Game::SimpleTrailedRocket::onTick(TankMatch *match) {
    SimpleRocket::onTick(match);

    if (!hasHit) {
        Vector2 from = position - match->gravity * gravityMult;
        Vector2 to = from + direction;
        to = match->checkForHit(position, to).second.round();
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



//
// RocketTrail
//

Hilltop::Game::RocketTrail::RocketTrail(int maxAge, ConsoleColor color) : Entity(), color(color) {
    maxEntityAge = maxAge;
    gravityMult = 0.0f;
}

std::shared_ptr<RocketTrail> Hilltop::Game::RocketTrail::create(int maxAge, ConsoleColor color) {
    return std::shared_ptr<RocketTrail>(new RocketTrail(maxAge, color));
}

void Hilltop::Game::RocketTrail::onDraw(TankMatch *match, Console::DoublePixelBufferedConsole &console) {
    Entity::onDraw(match, console);

    Vector2 p = position.round();
    console.set(p.X, p.Y, color);
}



//
// Explosion
//

Hilltop::Game::Explosion::Explosion(int size, int damage) : Entity(), size(size), damage(damage) {
    gravityMult = 0.0f;
    coreSize = size - 1;
}

void Hilltop::Game::Explosion::destroyLand(TankMatch *match) {
    Vector2 p = position.round();
    for (int i = p.X - size; i <= p.X + size; i++)
        for (int j = p.Y - size; j <= p.Y + size; j++)
            if (distance(Vector2(i, j), p) < size)
                match->set(i, j, AIR);
}

void Hilltop::Game::Explosion::createLand(TankMatch *match) {
    Vector2 p = position.round();
    for (int i = p.X - size; i <= p.X + size; i++)
        for (int j = p.Y - size; j <= p.Y + size; j++)
            if (distance(Vector2(i, j), p) < size)
                if (match->get(i, j) == AIR)
                    match->set(i, j, DIRT);
}

std::shared_ptr<Explosion> Hilltop::Game::Explosion::create(int size, int damage) {
    return std::shared_ptr<Explosion>(new Explosion(size, damage));
}

void Hilltop::Game::Explosion::onTick(TankMatch *match) {
    Entity::onTick(match);

    if (entityAge >= 2) {
        if (willDestroyLand) {
            willDestroyLand = false;
            destroyLand(match);
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

void Hilltop::Game::Explosion::onDraw(TankMatch *match, Console::DoublePixelBufferedConsole &console) {
    Entity::onDraw(match, console);

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
}



//
// TankWheel
//

bool Hilltop::Game::TankWheel::enableDebug = false;

Hilltop::Game::TankWheel::TankWheel() : Entity() {}

std::shared_ptr<TankWheel> Hilltop::Game::TankWheel::create() {
    return std::shared_ptr<TankWheel>(new TankWheel());
}

void Hilltop::Game::TankWheel::onDraw(TankMatch *match, Console::DoublePixelBufferedConsole &console) {
    Entity::onDraw(match, console);

    if (!enableDebug)
        return;

    Vector2 p = position.round();
    if (entityAge % 2 == 0)
        console.set(p.X, p.Y, RED);
}



//
// Tank
//

Hilltop::Game::Tank::Tank(ConsoleColor color, ConsoleColor barrelColor)
    : Entity(), color(color), barrelColor(barrelColor) {
    gravityMult = 0.0f;
}

Vector2 Hilltop::Game::Tank::getBarrelEnd() {
    static const float ds2 = std::sqrtf(2.0) * 2.0f;

    float a = angle * PI / 180.0f;
    Vector2 p = position.round();

    Vector2 c = { std::sinf(a), std::cosf(a) };
    Vector2 c2 = { c.X * c.X, c.Y * c.Y };
    Vector2 subterm = { 2.0f + c2.X - c2.Y, 2.0f - c2.X + c2.Y };
    Vector2 term1 = { subterm.X + c.X * ds2, subterm.Y + c.Y * ds2 };
    Vector2 term2 = { subterm.X - c.X * ds2, subterm.Y - c.Y * ds2 };
    Vector2 r = { 0.5f * (std::sqrtf(term1.X) - std::sqrtf(term2.X)),
        0.5f * (std::sqrtf(term1.Y) - std::sqrtf(term2.Y)) };

    return p + r * 2.0f;
}

Vector2 Hilltop::Game::Tank::calcTrajectory(int angle, int power) {
    const static float pi = std::atanf(1) * 4;
    float ang = (float)angle * pi / 180;
    return Vector2(-std::sinf(ang), std::cosf(ang)) * 8.0f * ((float)power / 100.0f);
}

std::shared_ptr<Tank> Hilltop::Game::Tank::create(ConsoleColor color) {
    return std::shared_ptr<Tank>(new Tank(color, color));
}

std::shared_ptr<Tank> Hilltop::Game::Tank::create(ConsoleColor color, ConsoleColor barrelColor) {
    return std::shared_ptr<Tank>(new Tank(color, barrelColor));
}

void Hilltop::Game::Tank::initWheels(TankMatch *match) {
    for (int i = 0; i < 5; i++) {
        if (!wheels[i]) {
            wheels[i] = TankWheel::create();
            wheels[i]->position = { position.X, position.Y + i };
        }
        match->addEntity(*wheels[i]);
    }
}

void Hilltop::Game::Tank::onTick(TankMatch *match) {
    Entity::onTick(match);

    float top = wheels[0]->position.X;
    Vector2 leastDir = wheels[0]->direction;
    for (int i = 1; i < 5; i++) {
        top = std::min(top, wheels[i]->position.X);
        if (wheels[i]->direction.X < leastDir.X)
            leastDir = wheels[i]->direction;
    }
    position.X = top;

    for (int i = 0; i < 5; i++) {
        wheels[i]->position = { position.X, position.Y + i };
        wheels[i]->direction = leastDir;
    }
}

void Hilltop::Game::Tank::onDraw(TankMatch *match, Console::DoublePixelBufferedConsole &console) {
    const static float pi = std::atanf(1) * 4;

    Entity::onDraw(match, console);

    Vector2 p = position.round();
    
    // draw barrel
    float a = angle * pi / 180.0f;
    foreachPixel(p, getBarrelEnd(), [this, &console](Vector2 v)->bool {
        console.set(v.X - 2, v.Y + 2, barrelColor);
        return false;
    });

    // draw the rest of the tank
    for (int i = 0; i < 5; i++)
        console.set(p.X - 1, p.Y + i, color);
    for (int i = 1; i < 4; i++)
        console.set(p.X - 2, p.Y + i, color);
}



//
// TankMatch
//

static const ConsoleColor LAND_COLORS[NUM_LAND_TYPES] = { DARK_BLUE, DARK_GREEN, BROWN, BLACK };

void Hilltop::Game::TankMatch::doEntityTick() {
    for (const std::shared_ptr<Entity> &p : entities) {
        p->onTick(this);
    }

    while (!entityChanges.empty()) {
        std::pair<bool, std::shared_ptr<Entity>> ev = entityChanges.front();
        entityChanges.pop();

        std::vector<std::shared_ptr<Entity>>::iterator it =
            std::find(entities.begin(), entities.end(), ev.second);
        bool exists = it != entities.end();

        if (ev.first) {
            if (!exists)
                entities.push_back(ev.second);
        } else {
            if (exists)
                entities.erase(it);
        }
    }

    for (const std::shared_ptr<Entity> &p : entities) {
        Vector2 oldPos = p->position;
        Vector2 newPos = oldPos + p->direction;
        std::pair<bool, Vector2> hit = checkForHit(oldPos, newPos, p->groundHog);
        p->position = hit.second;
        if (hit.first)
            p->onHit(this);

        p->direction = p->direction + gravity * p->gravityMult;

        if (p->position.Y < 0 || p->position.Y >= width || p->position.X > height + 1)
            removeEntity(*p);
    }
}

bool Hilltop::Game::TankMatch::doLandPhysics() {
    bool ret = false;
    for (int i = height - 1; i > 0; i--) {
        for (int j = 0; j < width; j++) {
            LandType top = get(i - 1, j);
            LandType bottom = get(i, j);
            if (bottom == AIR && top != AIR) {
                set(i, j, top);
                set(i - 1, j, AIR);
                ret = true;
            }
        }
    }
    return ret;
}

LandType Hilltop::Game::TankMatch::get(int x, int y) {
    if (x < 0 || x >= height || y < 0 || y >= width) {
        if (x >= height)
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
    : width(width), height(height), map(width * height), canvas(width, height) {}

void Hilltop::Game::TankMatch::addEntity(Entity &entity) {
    entityChanges.push(make_pair(true, entity.shared_from_this()));
}

void Hilltop::Game::TankMatch::removeEntity(Entity &entity) {
    entityChanges.push(make_pair(false, entity.shared_from_this()));
}

void Hilltop::Game::TankMatch::buildMap(std::function<float(float)> generator) {
    for (int i = 0; i < width; i++) {
        int val = height - generator((float)i / width) * height;
        for (int j = height - 1; j >= val; j--)
            set(j, i, GRASS);
    }
}

std::pair<bool, Vector2> Hilltop::Game::TankMatch::checkForHit(const Vector2 from, const Vector2 to,
    bool groundHog) {
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

void Hilltop::Game::TankMatch::draw(Console::Console &console) {
    canvas.clear(LAND_COLORS[AIR]);

    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            LandType land = get(i, j);
            if (land != AIR)
                canvas.set(i, j, LAND_COLORS[land]);
        }
    }

    for (const std::shared_ptr<Entity> &p : entities) {
        p->onDraw(this, canvas);
    }

    canvas.commit(console);
}

void Hilltop::Game::TankMatch::doTick(uint64_t tickNumber) {
    if (timeSinceLast == timeBetweenRocket) {
        timeSinceLast = 0;

        std::shared_ptr<SimpleRocket> rocket = SimpleTrailedRocket::create(WHITE, DARK_GRAY, 3);
        rocket->position = { 20, (float)width / 2 };
        int angle = scale(rand(), 0, RAND_MAX, 0, 359);
        int power = scale(rand(), 0, RAND_MAX, 20, 80);
        rocket->direction = Tank::calcTrajectory(angle, power);
        if ((int)scale(rand(), 0, RAND_MAX, 0, 2) == 0) {
            rocket->destroyLand = false;
            rocket->createLand = true;
            rocket->color = BROWN;
        }
        addEntity(*rocket);
    }
    timeSinceLast++;

    if (tickNumber % 3 == 0)
        doLandPhysics();
    
    doEntityTick();
}
