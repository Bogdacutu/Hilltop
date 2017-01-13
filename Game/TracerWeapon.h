#pragma once

#include "Game/Weapon.h"


namespace Hilltop {
namespace Game {

class TracerWeapon : public Weapon {
private:
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive &ar, const unsigned int version) {
        ar & boost::serialization::base_object<Weapon>(*this);
    }

public:
    static const int TRACER_INTERVAL = 5;
    static const int TRACER_OFFSET = TRACER_INTERVAL * 3;

    TracerWeapon();

    virtual void fire(TankMatch &match, int playerNumber) override;
};

}
}
