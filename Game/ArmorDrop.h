#pragma once

#include "Game/Drop.h"


namespace Hilltop {
namespace Game {

class ArmorDrop : public Drop {
private:
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive &ar, const unsigned int version) {
        ar & boost::serialization::base_object<Drop>(*this);
    }

protected:
    ArmorDrop();

public:
    static const int ARMOR = 25;

    static std::shared_ptr<ArmorDrop> create();

    virtual void handleTank(TankMatch *match, Tank &tank) override;
};

}
}
