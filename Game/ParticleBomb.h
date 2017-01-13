#pragma once

#include "Game/SimpleTrailedRocket.h"


namespace Hilltop {
namespace Game {

class ParticleBomb : public SimpleTrailedRocket {
private:
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive &ar, const unsigned int version) {
        ar & boost::serialization::base_object<SimpleTrailedRocket>(*this);
        ar & team;
    }

protected:
    ParticleBomb();

public:
    static const int TRIGGER_DISTANCE_X = 40;
    static const int TRIGGER_DISTANCE_Y = 8;
    static const int STEPS = 18;

    static std::shared_ptr<ParticleBomb> create();

    int team = -1;

    virtual void onTick(TankMatch *match) override;
};

}
}
