#pragma once

#include "Game/SimpleRocket.h"


namespace Hilltop {
namespace Game {

class SimpleTrailedRocket : public SimpleRocket {
private:
    friend class boost::serialization::access;
    template<class Archive>
    friend inline void load_construct_data(Archive &ar, SimpleTrailedRocket *t, const unsigned int);
    template<class Archive>
    void serialize(Archive &ar, const unsigned int version) {
        ar & boost::serialization::base_object<SimpleRocket>(*this);
        ar & trailColor;
        ar & trailTime;
    }

protected:
    SimpleTrailedRocket(Console::ConsoleColor color, Console::ConsoleColor trailColor, int trailTime);

public:
    Console::ConsoleColor trailColor;
    int trailTime;

    static std::shared_ptr<SimpleTrailedRocket> create(Console::ConsoleColor color,
        Console::ConsoleColor trailColor, int trailTime);

    virtual void onTick(TankMatch *match) override;
};

template<class Archive>
inline void load_construct_data(Archive &ar, SimpleTrailedRocket *t, const unsigned int) {
    ::new(t) SimpleTrailedRocket(ConsoleColor(), ConsoleColor(), 0);
}

}
}
