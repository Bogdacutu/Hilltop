#pragma once

#include "Game/Drop.h"


namespace Hilltop {
namespace Game {

class HealthDrop : public Drop {
private:
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive &ar, const unsigned int version) {
        ar & boost::serialization::base_object<Drop>(*this);
    }

protected:
    HealthDrop();

public:
    static const int HEALTH = 50;

    static std::shared_ptr<HealthDrop> create();

    virtual void handleTank(TankMatch *match, Tank &tank) override;
};

}
}
