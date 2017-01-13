#pragma once

#include "Game/RocketWeapon.h"


namespace Hilltop {
namespace Game {

class BouncyRocketWeapon : public RocketWeapon {
private:
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive &ar, const unsigned int version) {
        ar & boost::serialization::base_object<RocketWeapon>(*this);
    }

protected:
    virtual std::shared_ptr<Entity> createRocket(Vector2 position, Vector2 direction) override;

public:
    BouncyRocketWeapon();
};

}
}
