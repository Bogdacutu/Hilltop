#pragma once

#include "Game/Weapon.h"


namespace Hilltop {
namespace Game {

class RocketWeapon : public Weapon {
private:
    friend class boost::serialization::access;
    template<class Archive>
    friend inline void load_construct_data(Archive &ar, RocketWeapon *t, const unsigned int);
    template<class Archive>
    void serialize(Archive &ar, const unsigned int version) {
        ar & boost::serialization::base_object<Weapon>(*this);
        ar & explosionSize;
    }

protected:
    virtual std::shared_ptr<Entity> createRocket(Vector2 position, Vector2 direction);

public:
    RocketWeapon(int numRockets);

    const int numRockets;
    int explosionSize = 5;

    virtual void fire(TankMatch &match, int playerNumber) override;
};

template<class Archive>
inline void save_construct_data(Archive &ar, const RocketWeapon *t, const unsigned int) {
    ar << t->numRockets;
}

template<class Archive>
inline void load_construct_data(Archive &ar, RocketWeapon *t, const unsigned int) {
    int numRockets;
    ar >> numRockets;
    ::new(t) RocketWeapon(numRockets);
}

}
}
