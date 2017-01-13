#pragma once

#include "Game/Weapon.h"


namespace Hilltop {
namespace Game {

class ParticleBombWeapon : public Weapon {
private:
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive &ar, const unsigned int version) {
        ar & boost::serialization::base_object<Weapon>(*this);
    }

public:
    ParticleBombWeapon();

    virtual void fire(TankMatch &match, int playerNumber) override;
};

}
}
