#pragma once

#include "Game/Drop.h"


namespace Hilltop {
namespace Game {

class WeaponDrop : public Drop {
private:
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive &ar, const unsigned int version) {
        ar & boost::serialization::base_object<Drop>(*this);
    }

protected:
    WeaponDrop();

public:
    static const int WEAPONS = 10;

    static std::shared_ptr<WeaponDrop> create();

    virtual void handleTank(TankMatch *match, Tank &tank) override;
};

}
}
