#pragma once

#include "Game/RocketWeapon.h"


namespace Hilltop {
namespace Game {

class GroundRocketWeapon : public RocketWeapon {
private:
    friend class boost::serialization::access;
    template<class Archive>
    friend inline void load_construct_data(Archive &ar, GroundRocketWeapon *t, const unsigned int);
    template<class Archive>
    void serialize(Archive &ar, const unsigned int version) {
        ar & boost::serialization::base_object<RocketWeapon>(*this);
    }

protected:
    virtual std::shared_ptr<Entity> createRocket(Vector2 position, Vector2 direction) override;

public:
    GroundRocketWeapon(int numRockets);
};

template<class Archive>
inline void save_construct_data(Archive &ar, const GroundRocketWeapon *t, const unsigned int) {
    ar << t->numRockets;
}

template<class Archive>
inline void load_construct_data(Archive &ar, GroundRocketWeapon *t, const unsigned int) {
    int numRockets;
    ar >> numRockets;
    ::new(t) GroundRocketWeapon(numRockets);
}

}
}
