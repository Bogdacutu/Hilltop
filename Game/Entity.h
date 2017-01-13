#pragma once

#include "Game/Vector2.h"
#include "Console/BufferedConsole.h"
#include "Console/DoublePixelBufferedConsole.h"
#include <memory>


namespace Hilltop {
namespace Game {

class TankMatch;

class Entity : public std::enable_shared_from_this<Entity> {
private:
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive &ar, const unsigned int version) {
        ar & position;
        ar & direction;
        ar & gravityMult;
        ar & groundHog;
        ar & hasHit;
        ar & hasExpired;
        ar & entityAge;
        ar & maxEntityAge;
        ar & physicsSpeed;
    }

protected:
    Entity();

public:
    Vector2 position = { -1.0f, -1.0f };
    Vector2 direction = { 0.0f, 0.0f };
    float gravityMult = 1.0f;

    bool groundHog = false;
    bool hasHit = false;
    bool hasExpired = false;
    int entityAge = 0;
    int maxEntityAge = -1;
    int physicsSpeed = 1;

    virtual ~Entity();
    static std::shared_ptr<Entity> create();

    virtual void onTick(TankMatch *match);
    virtual void onDraw(TankMatch *match, Console::DoublePixelBufferedConsole &console);
    virtual void onDirectDraw(TankMatch *match, Console::BufferedConsole &console);
    virtual void onHit(TankMatch *match);
    virtual void onExpire(TankMatch *match);
};

}
}
