#pragma once

#include "Game/Entity.h"


namespace Hilltop {
namespace Game {

class RocketTrail : public Entity {
private:
    friend class boost::serialization::access;
    template<class Archive>
    friend inline void load_construct_data(Archive &ar, RocketTrail *t, const unsigned int);
    template<class Archive>
    void serialize(Archive &ar, const unsigned int version) {
        ar & boost::serialization::base_object<Entity>(*this);
        ar & color;
    }

protected:
    RocketTrail(int maxAge, Console::ConsoleColor color);

public:
    Console::ConsoleColor color;

    static std::shared_ptr<RocketTrail> create(int maxAge, Console::ConsoleColor color);

    virtual void onDraw(TankMatch *match, Console::DoublePixelBufferedConsole &console) override;
};

template<class Archive>
inline void load_construct_data(Archive &ar, RocketTrail *t, const unsigned int) {
    ::new(t) RocketTrail(0, ConsoleColor());
}

}
}
