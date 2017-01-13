#pragma once

#include "Game/Entity.h"
#include "Game/Weapon.h"
#include <boost/serialization/split_member.hpp>
#include <queue>


namespace Hilltop {
namespace Game {

class TankController;

class TankMatch {
public:
    enum LandType : unsigned char {
        AIR = 0,
        GRASS,
        DIRT,
        NUM_LAND_TYPES,
    };

    friend class boost::serialization::access;
    template<class Archive>
    void save(Archive &ar, const unsigned int version) const {
        ar & entities;
        ar & entityChanges;
        ar & recentUpdateResult;
        ar & players;
        ar & currentPlayer;
        ar & gravity;
        ar & tickNumber;
        ar & isAiming;
        ar & gameOver;
        ar & firingMode;

        if (map.size() > 0) {
            LandType lastType = map[0];
            size_t spanLength = 1;
            for (int i = 1; i < map.size(); i++) {
                LandType type = map[i];
                if (type != lastType) {
                    ar & lastType;
                    ar & spanLength;
                    lastType = type;
                    spanLength = 1;
                } else {
                    spanLength++;
                }
            }
            ar & lastType;
            ar & spanLength;
        }
    }
    template<class Archive>
    void load(Archive &ar, const unsigned int version) {
        ar & entities;
        ar & entityChanges;
        ar & recentUpdateResult;
        ar & players;
        ar & currentPlayer;
        ar & gravity;
        ar & tickNumber;
        ar & isAiming;
        ar & gameOver;
        ar & firingMode;

        LandType type;
        size_t spanLength = 0;
        for (int i = 0; i < map.size(); i++) {
            if (spanLength == 0) {
                ar & type;
                ar & spanLength;
            }
            map[i] = type;
            spanLength--;
        }
    }
    BOOST_SERIALIZATION_SPLIT_MEMBER()

    std::vector<LandType> map;

    std::vector<std::shared_ptr<Entity>> entities;
    std::queue<std::pair<bool, std::shared_ptr<Entity>>> entityChanges;

    static const int RECENT_UPDATE_COUNT = 10;
    bool recentUpdateResult[10] = {};

    static const int AIM_RETICLE_TIME = 6;
    static const int LAND_PHYSICS_EVERY_TICKS = 3;

    bool doEntityTick();
    bool doLandPhysics();

public:
    enum FiringMode {
        FIRE_SOLO,
        FIRE_AS_TEAM,
        FIRE_EVERYTHING,
    };

    static const int UNLIMITED_WEAPON_THRESHOLD = 99;

    static const int DEFAULT_MATCH_WIDTH = 180;
    static const int DEFAULT_MATCH_HEIGHT = 90;

    static const int AIRDROP_EVERY_TURNS = 12;

    const unsigned short width, height;
    Console::DoublePixelBufferedConsole canvas;

    std::vector<std::shared_ptr<TankController>> players;
    int currentPlayer = 0;

    Vector2 gravity = { 0.15f, 0.0f };
    bool updateMattered = false;
    uint64_t tickNumber = 0;
    bool isAiming = true;
    bool gameOver = false;
    bool shownGameOver = false;
    unsigned short lowestAir;
    unsigned short highestLand;

    FiringMode firingMode = FIRE_AS_TEAM;

    static std::vector<std::shared_ptr<Weapon>> weapons;
    static void initalizeWeapons();

    TankMatch();
    TankMatch(unsigned short width, unsigned short height);

    LandType get(int x, int y);
    void set(int x, int y, LandType type);

    void addEntity(Entity &entity);
    void removeEntity(Entity &entity);

    void buildMap(std::function<float(float)> generator);
    void arrangeTanks();
    std::pair<bool, Vector2> checkForHit(const Vector2 from, const Vector2 to, bool groundHog = false);
    void doAirdrop();

    void draw(Console::BufferedConsole &console);

    void tick();
    bool recentUpdatesMattered();
    void fire(int playerNumber);
    void fire();

    int getNextPlayer();
};

template<class Archive>
inline void save_construct_data(Archive &ar, const TankMatch *t, const unsigned int) {
    ar << t->width;
    ar << t->height;
}

template<class Archive>
inline void load_construct_data(Archive &ar, TankMatch *t, const unsigned int) {
    unsigned short width, height;
    ar >> width;
    ar >> height;
    ::new(t) TankMatch(width, height);
}

}
}
