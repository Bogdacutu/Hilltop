#include "Game/TankMatch.h"
#include "Game/ArmorDrop.h"
#include "Game/BouncyRocketWeapon.h"
#include "Game/BulletRainWeapon.h"
#include "Game/DirtRocketWeapon.h"
#include "Game/GroundRocketWeapon.h"
#include "Game/HealthDrop.h"
#include "Game/MinigunWeapon.h"
#include "Game/ParticleBombWeapon.h"
#include "Game/RocketWeapon.h"
#include "Game/TankController.h"
#include "Game/TracerWeapon.h"
#include "Game/WeaponDrop.h"


namespace Hilltop {
namespace Game {

static const Console::ConsoleColor LAND_COLORS[TankMatch::NUM_LAND_TYPES] =
    { Console::DARK_BLUE, Console::DARK_GREEN, Console::BROWN };

bool TankMatch::doEntityTick() {
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

        for (int i = 0; i < p->physicsSpeed; i++) {
            Vector2 oldPos = p->position;
            Vector2 newPos = oldPos + p->direction;
            std::pair<bool, Vector2> hit = checkForHit(oldPos, newPos, p->groundHog);
            p->position = hit.second;
            if (hit.first)
                p->onHit(this);

            if (oldPos.round() != p->position.round())
                ret = true;

            p->direction = p->direction + gravity * p->gravityMult;
        }

        Vector2 pos = p->position.round();
        if (pos.Y < 0 || pos.Y >= width || pos.X > height + 1) {
            p->onExpire(this);
            removeEntity(*p);
        }
    }

    return ret;
}

bool TankMatch::doLandPhysics() {
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

std::vector<std::shared_ptr<Weapon>> TankMatch::weapons;

void TankMatch::initalizeWeapons() {
    {
        std::shared_ptr<RocketWeapon> weapon = std::make_shared<RocketWeapon>(1);
        weapon->name = "Ordinary Missile";
        weapons.push_back(weapon);
    }

    {
        std::shared_ptr<RocketWeapon> weapon = std::make_shared<RocketWeapon>(3);
        weapon->name = "Triple Missile";
        weapon->explosionSize = 4;
        weapons.push_back(weapon);
    }

    {
        std::shared_ptr<RocketWeapon> weapon = std::make_shared<RocketWeapon>(5);
        weapon->name = "Five Missiles";
        weapon->explosionSize = 4;
        weapons.push_back(weapon);
    }

    {
        std::shared_ptr<DirtRocketWeapon> weapon = std::make_shared<DirtRocketWeapon>(1);
        weapon->name = "Dirt Missile";
        weapon->explosionSize = 6;
        weapons.push_back(weapon);
    }

    {
        std::shared_ptr<GroundRocketWeapon> weapon = std::make_shared<GroundRocketWeapon>(1);
        weapon->name = "Ground Missile";
        weapons.push_back(weapon);
    }

    {
        std::shared_ptr<BouncyRocketWeapon> weapon = std::make_shared<BouncyRocketWeapon>();
        weapon->name = "Bouncy Missile";
        weapon->explosionSize = 6;
        weapons.push_back(weapon);
    }

    {
        std::shared_ptr<TracerWeapon> weapon = std::make_shared<TracerWeapon>();
        weapon->name = "Tracers";
        weapons.push_back(weapon);
    }

    {
        std::shared_ptr<ParticleBombWeapon> weapon = std::make_shared<ParticleBombWeapon>();
        weapon->name = "Particle Bomb";
        weapons.push_back(weapon);
    }

    {
        std::shared_ptr<BulletRainWeapon> weapon = std::make_shared<BulletRainWeapon>();
        weapon->name = "Bullet Rain";
        weapons.push_back(weapon);
    }

    {
        std::shared_ptr<MinigunWeapon> weapon = std::make_shared<MinigunWeapon>();
        weapon->name = "Minigun";
        weapons.push_back(weapon);
    }

    for (const std::shared_ptr<Weapon> &weapon : weapons)
        if (weapon->name == Weapon::INVALID_NAME)
            __debugbreak();
}

TankMatch::LandType TankMatch::get(int x, int y) {
    if (x < 0 || x >= height || y < 0 || y >= width) {
        if (x >= height)
            return TankMatch::DIRT;
        else
            return TankMatch::AIR;
    }

    return map[x * width + y];
}

void TankMatch::set(int x, int y, LandType type) {
    if (x < 0 || x >= height || y < 0 || y >= width)
        return;

    map[x * width + y] = type;
}

TankMatch::TankMatch() : TankMatch(DEFAULT_MATCH_WIDTH, DEFAULT_MATCH_HEIGHT) {}

TankMatch::TankMatch(unsigned short width, unsigned short height)
    : width(width), height(height), map(width * height), canvas(width, height) {}

void TankMatch::addEntity(Entity &entity) {
    entityChanges.push(make_pair(true, entity.shared_from_this()));
}

void TankMatch::removeEntity(Entity &entity) {
    entityChanges.push(make_pair(false, entity.shared_from_this()));
}

void TankMatch::buildMap(std::function<float(float)> generator) {
    for (int i = 0; i < width; i++) {
        int val = height - generator((float)i / width) * height;
        for (int j = height - 1; j >= val; j--)
            set(j, i, GRASS);
    }
}

void TankMatch::arrangeTanks() {
    typedef std::pair<int, int> team_t;
    std::vector<team_t> teams;
    for (int i = 0; i < players.size(); i++) {
        bool found = false;
        for (team_t &team : teams) {
            if (players[i]->team == team.first) {
                team.second++;
                found = true;
                break;
            }
        }
        if (!found) {
            teams.push_back(std::make_pair(players[i]->team, 1));
        }
    }
    std::sort(teams.begin(), teams.end(), [](team_t x, team_t y)->bool {
        return x.second > y.second;
    });
    if (scale(rand(), 0, RAND_MAX, 0, 1) >= 0.5f)
        std::reverse(teams.begin(), teams.end());

    int leftBound = 10;
    int rightBound = width - 1 - 10 - 4;
    int index = 0;
    for (int i = 0; i < teams.size(); i++) {
        for (int j = 0; j < players.size(); j++) {
            if (players[j]->team == teams[i].first) {
                int left = scale(index++, 0, std::max(1, (int)players.size() - 1), leftBound, rightBound);
                players[j]->tank->position = Vector2(-1, left);
                players[j]->tank->initWheels(*this);

                if (left > width / 2)
                    players[j]->tank->angle = 135;
                else
                    players[j]->tank->angle = 45;
            }
        }
    }

    // settle tanks
    do {
        tick();
    } while (recentUpdatesMattered());
}

std::pair<bool, Vector2> TankMatch::checkForHit(const Vector2 from, const Vector2 to,
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

void TankMatch::doAirdrop() {
    int val = rand() % 3;
    std::shared_ptr<Drop> drop;
    switch (val) {
    case 0:
        drop = HealthDrop::create();
        break;
    case 1:
        drop = ArmorDrop::create();
        break;
    case 2:
        drop = WeaponDrop::create();
        break;
    }
    drop->position = { -10, (float)(rand() % width) };
    addEntity(*drop);
}

void TankMatch::draw(Console::BufferedConsole &console) {
    lowestAir = 0;
    highestLand = height - 1;

    canvas.clear(LAND_COLORS[AIR]);

    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            LandType land = get(i, j);
            if (land != AIR) {
                canvas.set(i, j, LAND_COLORS[land]);
                if (highestLand > i)
                    highestLand = i;
            } else {
                if (lowestAir < i)
                    lowestAir = i;
            }
        }
    }

    for (const std::shared_ptr<Entity> &p : entities) {
        p->onDraw(this, canvas);
    }

    if (isAiming && players[currentPlayer]->isHuman) {
        if (tickNumber % (AIM_RETICLE_TIME * 2) < AIM_RETICLE_TIME)
            players[currentPlayer]->tank->drawReticle(this, canvas);
    }

    canvas.commit(console);

    for (const std::shared_ptr<Entity> &p : entities) {
        p->onDirectDraw(this, console);
    }
}

void TankMatch::tick() {
    tickNumber++;
    updateMattered = false;

    if (tickNumber % LAND_PHYSICS_EVERY_TICKS == 0)
        updateMattered |= doLandPhysics();

    updateMattered |= doEntityTick();

    recentUpdateResult[tickNumber % RECENT_UPDATE_COUNT] = updateMattered;
}

bool TankMatch::recentUpdatesMattered() {
    for (int i = 0; i < RECENT_UPDATE_COUNT; i++)
        if (recentUpdateResult[i])
            return true;
    return false;
}

void TankMatch::fire(int playerNumber) {
    std::shared_ptr<TankController> player = players[playerNumber];
    std::pair<std::shared_ptr<Weapon>, int> &weapon = player->weapons[player->currentWeapon];
    weapon.first->fire(*this, playerNumber);
    if (weapon.second < UNLIMITED_WEAPON_THRESHOLD)
        weapon.second--;
}

void TankMatch::fire() {
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
                if (players[i]->team == players[currentPlayer]->team && players[i]->tank->alive)
                    fire(i);
    } else if (firingMode == FIRE_EVERYTHING) {
        if (currentPlayer == players.size() - 1)
            for (int i = 0; i < players.size(); i++)
                if (players[i]->tank->alive)
                    fire(i);
    }
}

int TankMatch::getNextPlayer() {
    int currentTeam = players[currentPlayer]->team;
    int minTeam = currentTeam;
    int maxTeam = currentTeam;
    int minAliveTeam = -1;
    int maxAliveTeam = -1;

    for (int i = 0; i < players.size(); i++) {
        int team = players[i]->team;
        if (team < minTeam)
            minTeam = team;
        if (team > maxTeam)
            maxTeam = team;
        if (players[i]->tank->alive) {
            if (team < minAliveTeam || minAliveTeam == -1)
                minAliveTeam = team;
            if (team > maxAliveTeam)
                maxAliveTeam = team;
        }
    }

    if (minAliveTeam == maxAliveTeam && minTeam != maxTeam)
        return -1;

    for (int i = currentPlayer + 1; i < players.size(); i++)
        if (players[i]->team == currentTeam && players[i]->tank->alive)
            return i;

    for (currentTeam++; currentTeam <= maxAliveTeam; currentTeam++)
        for (int i = 0; i < players.size(); i++)
            if (players[i]->team == currentTeam && players[i]->tank->alive)
                return i;

    for (int i = 0; i < players.size(); i++)
        if (players[i]->team == minAliveTeam && players[i]->tank->alive)
            return i;

    return -1; // fallback
}

}
}
