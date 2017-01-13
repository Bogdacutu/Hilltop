#pragma once

#include "Game/Entity.h"


namespace Hilltop {
namespace Game {

class SimpleRocket : public Entity {
private:
    friend class boost::serialization::access;
    template<class Archive>
    friend inline void load_construct_data(Archive &ar, SimpleRocket *t, const unsigned int);
    template<class Archive>
    void serialize(Archive &ar, const unsigned int version) {
        ar & boost::serialization::base_object<Entity>(*this);
        ar & color;
        ar & explosionSize;
        ar & explosionDamage;
        ar & destroyLand;
        ar & createLand;
    }

protected:
    SimpleRocket(Console::ConsoleColor color);

public:
    Console::ConsoleColor color;

    int explosionSize = 5;
    float explosionDamage = 1.0f;

    bool destroyLand = true;
    bool createLand = false;

    static std::shared_ptr<SimpleRocket> create(Console::ConsoleColor color);

    virtual void onDraw(TankMatch *match, Console::DoublePixelBufferedConsole &console) override;
    virtual void onHit(TankMatch *match) override;
};

template<class Archive>
inline void load_construct_data(Archive &ar, SimpleRocket *t, const unsigned int) {
    ::new(t) SimpleRocket(ConsoleColor());
}

}
}
