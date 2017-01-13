#pragma once

#include "Game/Entity.h"
#include "Game/Tank.h"


namespace Hilltop {
namespace Game {

class Minigun : public Entity {
private:
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive &ar, const unsigned int version) {
        ar & boost::serialization::base_object<Entity>(*this);
        ar & tank;
    }

protected:
    Minigun();

public:
    static const int MINIGUN_TICKS = 32;
    static const int ANGLE_OFFSET = 3;

    std::shared_ptr<Tank> tank;

    static std::shared_ptr<Minigun> create();

    virtual void onTick(TankMatch *match) override;
};

}
}
