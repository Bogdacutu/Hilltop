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
                match->set(i, j, TankMatch::AIR);
}

void Hilltop::Game::Explosion::createLand(TankMatch *match) {
    Vector2 p = position.round();
    for (int i = p.X - size; i <= p.X + size; i++)
        for (int j = p.Y - size; j <= p.Y + size; j++)
            if (distance(Vector2(i, j), p) < size)
                if (match->get(i, j) == TankMatch::AIR)
                    match->set(i, j, TankMatch::DIRT);
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
// Tracer
//

Hilltop::Game::Tracer::Tracer() {}

std::shared_ptr<Tracer> Hilltop::Game::Tracer::create() {
    return std::shared_ptr<Tracer>(new Tracer());
}

void Hilltop::Game::Tracer::onTick(TankMatch *match) {
    Entity::onTick(match);

    match->updateMattered = true;
}

void Hilltop::Game::Tracer::onDraw(TankMatch *match, Console::DoublePixelBufferedConsole &console) {
    Entity::onDraw(match, console);

    if (hasHit) {
        // TODO
    }
}

void Hilltop::Game::Tracer::onHit(TankMatch *match) {
    if (!hasHit) {
        maxEntityAge = entityAge + DURATION;
        gravityMult = 0.0f;
    }

    Entity::onHit(match);
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

Hilltop::Game::Tank::Tank(ConsoleColor color) : Entity(), color(color) {
    gravityMult = 0.0f;
}

Vector2 Hilltop::Game::Tank::getBarrelBase() {
    return position.round() + Vector2(-2.0f, 2.0f);
}

Vector2 Hilltop::Game::Tank::getBarrelEnd() {
    static const float DS2 = std::sqrtf(2.0) * 2.0f;

    float a = angle * PI / 180.0f;
    Vector2 p = getBarrelBase();

    Vector2 c = { -std::sinf(a), std::cosf(a) };
    Vector2 c2 = { c.X * c.X, c.Y * c.Y };
    Vector2 subterm = { 2.0f + c2.X - c2.Y, 2.0f - c2.X + c2.Y };
    Vector2 term1 = { subterm.X + c.X * DS2, subterm.Y + c.Y * DS2 };
    Vector2 term2 = { subterm.X - c.X * DS2, subterm.Y - c.Y * DS2 };
    Vector2 r = Vector2(std::sqrtf(term1.X) - std::sqrtf(term2.X),
        std::sqrtf(term1.Y) - std::sqrtf(term2.Y)) * 0.5f;

    return p + r * 2.0f;
}

Vector2 Hilltop::Game::Tank::getProjectileBase() {
    Vector2 r = getBarrelEnd().round();
    Vector2 p = (r - getBarrelBase()).round();

    if (p.X >= 1.0f)
        r.X += 1.0f;
    else if (p.X <= -1.0f)
        r.X -= 1.0f;

    if (p.Y >= 1.0f)
        r.Y += 1.0f;
    else if (p.Y <= -1.0f)
        r.Y -= 1.0f;

    return r;
}

Vector2 Hilltop::Game::Tank::calcTrajectory(int angle, int power) {
    float ang = (float)angle * PI / 180.0f;
    return Vector2(-std::sinf(ang), std::cosf(ang)) * 8.0f * ((float)power / 100.0f);
}

Vector2 Hilltop::Game::Tank::calcTrajectory() {
    return Tank::calcTrajectory(angle, power);
}

std::vector<Vector2> Hilltop::Game::Tank::getPixels() {
    std::vector<Vector2> pixels;

    // the barrel
    float a = angle * PI / 180.0f;
    foreachPixel(getBarrelBase(), getBarrelEnd(), [&pixels](Vector2 v)->bool {
        pixels.push_back(v);
        return false;
    });

    // the rest of the tank
    Vector2 p = position.round();
    for (int i = 0; i < 5; i++)
        pixels.push_back(p + Vector2(-1, i));
    for (int i = 1; i < 4; i++)
        pixels.push_back(p + Vector2(-2, i));

    return pixels;
}

bool Hilltop::Game::Tank::testCollision(Vector2 position) {
    position = position.round();
    int x = position.X;
    int y = position.Y;

    for (Vector2 &v : getPixels()) {
        int dx = v.X;
        int dy = v.Y;
        if (dx == x && dy == y)
            return true;
    }

    return false;
}

std::shared_ptr<Tank> Hilltop::Game::Tank::create(ConsoleColor color) {
    return std::shared_ptr<Tank>(new Tank(color));
}

void Hilltop::Game::Tank::initWheels(TankMatch &match) {
    for (int i = 0; i < 5; i++) {
        if (!wheels[i]) {
            wheels[i] = TankWheel::create();
        }
        wheels[i]->position = { position.X, position.Y + i };
        match.addEntity(*wheels[i]);
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

    for (Vector2 &v : getPixels())
        console.set(v.X, v.Y, color);

    Vector2 p = position.round();
    for (int i = 0; i < 9; i++)
        console.set(p.X + 2, p.Y - 2 + i, health > i * 11 ? GREEN : RED);
}

void Hilltop::Game::Tank::drawReticle(TankMatch *match, Console::DoublePixelBufferedConsole &console) {
    float ang = (float)angle * PI / 180.0f;
    Vector2 p = (getBarrelBase().round() + Vector2(-std::sinf(ang), std::cosf(ang)) * 10.0f).round();
    console.set(p.X, p.Y, WHITE);
}

bool Hilltop::Game::Tank::canMove(TankMatch *match, int direction) {
    Vector2 p = position;
    doMove(match, direction);
    if (p != position) {
        position = p;
        return true;
    } else {
        return false;
    }
}

void Hilltop::Game::Tank::doMove(TankMatch *match, int direction) {
    Vector2 p = position.round() + Vector2(0, direction);
    if (p.X < 0.0f || p.X >= match->width - 5.0f)
        return;

    bool ret = true;
    for (int i = 1; i <= 2; i++) {
        for (int j = 0; j < 5; j++) {
            if (match->get(p.X - i, p.Y + j) != TankMatch::AIR) {
                ret = false;
                break;
            }
        }
    }
    if (ret) {
        position = p;
    } else {
        ret = true;
        for (int i = 1; i <= 2; i++) {
            for (int j = 0; j < 5; j++) {
                if (match->get(p.X - i - 1, p.Y + j) != TankMatch::AIR) {
                    ret = false;
                    break;
                }
            }
        }
        if (ret)
            position = p + Vector2(-1.0f, 0.0f);
    }

    if (ret)
        initWheels(*match);
}



//
// Weapon
//

const std::string Hilltop::Game::Weapon::INVALID_NAME = "<invalid name>";

void Hilltop::Game::Weapon::fire(TankMatch &match, int playerNumber) {
    std::shared_ptr<TankController> player = match.players[playerNumber];
    std::shared_ptr<LastHitTracer> tracer = LastHitTracer::create(*player);
    tracer->position = player->tank->getProjectileBase();
    tracer->direction = player->tank->calcTrajectory();
    match.addEntity(*tracer);
}



//
// RocketWeapon
//

std::shared_ptr<SimpleRocket> Hilltop::Game::RocketWeapon::createRocket(Vector2 position,
    Vector2 direction) {
    std::shared_ptr<SimpleRocket> rocket = SimpleTrailedRocket::create(WHITE, DARK_GRAY, 3);
    rocket->position = position;
    rocket->direction = direction;
    return rocket;
}

Hilltop::Game::RocketWeapon::RocketWeapon(int numRockets) : Weapon(), numRockets(numRockets) {}

void Hilltop::Game::RocketWeapon::fire(TankMatch &match, int playerNumber) {
    Weapon::fire(match, playerNumber);

    std::shared_ptr<Tank> tank = match.players[playerNumber]->tank;
    std::shared_ptr<Entity> rocket = createRocket(tank->getProjectileBase(), tank->calcTrajectory());
    match.addEntity(*rocket);
}



//
// DirtRocketWeapon
//

std::shared_ptr<SimpleRocket> Hilltop::Game::DirtRocketWeapon::createRocket(Vector2 position,
    Vector2 direction) {
    std::shared_ptr<SimpleRocket> rocket = RocketWeapon::createRocket(position, direction);
    rocket->destroyLand = false;
    rocket->createLand = true;
    return rocket;
}

Hilltop::Game::DirtRocketWeapon::DirtRocketWeapon(int numRockets) : RocketWeapon(numRockets) {}



//
// TankController
//

Hilltop::Game::TankController::TankController() {}

std::shared_ptr<TankController> Hilltop::Game::TankController::create() {
    return std::shared_ptr<TankController>(new TankController());
}

void Hilltop::Game::TankController::addWeapon(std::shared_ptr<Weapon> weapon, int amount) {
    for (int i = 0; i < weapons.size(); i++) {
        if (weapons[i].first == weapon) {
            weapons[i].second += amount;
            return;
        }
    }

    weapons.push_back(std::make_pair(weapon, amount));
}

void Hilltop::Game::TankController::applyAI(TankController &player) {}



//
// LastHitTracer
//

Hilltop::Game::LastHitTracer::LastHitTracer(TankController &player) : player(player.shared_from_this()) {}

std::shared_ptr<LastHitTracer> Hilltop::Game::LastHitTracer::create(TankController &player) {
    return std::shared_ptr<LastHitTracer>(new LastHitTracer(player));
}

void Hilltop::Game::LastHitTracer::onTick(TankMatch *match) {
    Entity::onTick(match);

    player->lastHit = position.round();
    player->hasLastHit = true;
}

void Hilltop::Game::LastHitTracer::onDraw(TankMatch *match, Console::DoublePixelBufferedConsole &console) {
    Entity::onDraw(match, console);

    Vector2 p = position.round();
    if (!player->isHuman)
        console.set(p.X, p.Y, RED);
}



//
// TankMatch
//

static const ConsoleColor LAND_COLORS[TankMatch::NUM_LAND_TYPES] =
    { DARK_BLUE, DARK_GREEN, BROWN, BLACK };

bool Hilltop::Game::TankMatch::doEntityTick() {
    bool ret = false;

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
            if (!exists) {
                entities.push_back(ev.second);
                ret = true;
            }
        } else {
            if (exists) {
                entities.erase(it);
                ret = true;
            }
        }
    }

    for (const std::shared_ptr<Entity> &p : entities) {
        if (p->entityAge <= 0)
            continue;

        Vector2 oldPos = p->position;
        Vector2 newPos = oldPos + p->direction;
        std::pair<bool, Vector2> hit = checkForHit(oldPos, newPos, p->groundHog);
        p->position = hit.second;
        if (hit.first)
            p->onHit(this);

        if (oldPos.round() != p->position.round())
            ret = true;

        p->direction = p->direction + gravity * p->gravityMult;

        Vector2 pos = p->position.round();
        if (pos.Y < 0 || pos.Y >= width || pos.X > height + 1)
            removeEntity(*p);
    }

    return ret;
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

std::vector<std::shared_ptr<Weapon>> Hilltop::Game::TankMatch::weapons;

void Hilltop::Game::TankMatch::initalizeWeapons() {
    {
        std::shared_ptr<RocketWeapon> weapon = std::make_shared<RocketWeapon>(1);
        weapon->name = "Ordinary Missile";
        weapons.push_back(weapon);
    }

    {
        std::shared_ptr<DirtRocketWeapon> weapon = std::make_shared<DirtRocketWeapon>(1);
        weapon->name = "Dirty Missile";
        weapons.push_back(weapon);
    }

    for (const std::shared_ptr<Weapon> &weapon : weapons)
        if (weapon->name == Weapon::INVALID_NAME)
            __debugbreak();
}

TankMatch::LandType Hilltop::Game::TankMatch::get(int x, int y) {
    if (x < 0 || x >= height || y < 0 || y >= width) {
        if (x >= height)
            return TankMatch::DIRT;
        else
            return TankMatch::AIR;
    }

    return map[x * width + y];
}

void Hilltop::Game::TankMatch::set(int x, int y, LandType type) {
    if (x < 0 || x >= height || y < 0 || y >= width)
        return;

    map[x * width + y] = type;
}

Hilltop::Game::TankMatch::TankMatch() : TankMatch(DEFAULT_MATCH_WIDTH, DEFAULT_MATCH_HEIGHT) {}

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

void Hilltop::Game::TankMatch::arrangeTanks() {
    int leftBound = 10;
    int rightBound = width - 1 - 10 - 4;
    for (int i = 0; i < players.size(); i++) {
        int left = scale(i, 0, std::max(1, (int)players.size() - 1), leftBound, rightBound);
        players[i]->tank->position = Vector2(-1, left);
        players[i]->tank->initWheels(*this);

        if (left > width / 2)
            players[i]->tank->angle = 135;
        else
            players[i]->tank->angle = 45;
    }

    // settle tanks
    do {
        tick();
    } while (recentUpdatesMattered());
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

    if (isAiming) {
        if (tickNumber % (AIM_RETICLE_TIME * 2) < AIM_RETICLE_TIME)
            players[currentPlayer]->tank->drawReticle(this, canvas);
    }

    canvas.commit(console);
}

void Hilltop::Game::TankMatch::tick() {
    tickNumber++;
    updateMattered = false;

    if (tickNumber % LAND_PHYSICS_EVERY_TICKS == 0)
        updateMattered |= doLandPhysics();
    
    updateMattered |= doEntityTick();

    recentUpdateResult[tickNumber % RECENT_UPDATE_COUNT] = updateMattered;
}

bool Hilltop::Game::TankMatch::recentUpdatesMattered() {
    for (int i = 0; i < RECENT_UPDATE_COUNT; i++)
        if (recentUpdateResult[i])
            return true;
    return false;
}

void Hilltop::Game::TankMatch::fire(int playerNumber) {
    std::shared_ptr<TankController> player = players[playerNumber];
    std::pair<std::shared_ptr<Weapon>, int> &weapon = player->weapons[player->currentWeapon];
    weapon.first->fire(*this, playerNumber);
    if (weapon.second < UNLIMITED_WEAPON_THRESHOLD)
        weapon.second--;
}

void Hilltop::Game::TankMatch::fire() {
    if (firingMode == FIRE_SOLO) {
        fire(currentPlayer);
    } else if (firingMode == FIRE_AS_TEAM) {
        bool found = false;
        for (int i = currentPlayer + 1; i < players.size(); i++) {
            if (players[i]->team == players[currentPlayer]->team) {
                found = true;
                break;
            }
        }

        if (!found)
            for (int i = 0; i < players.size(); i++)
                if (players[i]->team == players[currentPlayer]->team)
                    fire(i);
    } else if (firingMode == FIRE_EVERYTHING) {
        if (currentPlayer == players.size() - 1)
            for (int i = 0; i < players.size(); i++)
                fire(i);
    }
}

int Hilltop::Game::TankMatch::getNextPlayer() {
    int currentTeam = players[currentPlayer]->team;
    
    for (int i = currentPlayer + 1; i < players.size(); i++)
        if (players[i]->team == currentTeam)
            return i;

    currentTeam++;

    for (int i = 0; i < players.size(); i++)
        if (players[i]->team == currentTeam)
            return i;

    for (int i = 0; i < players.size(); i++)
        if (players[i]->team == 1)
            return i;

    return 0; // fallback
}
