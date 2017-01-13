#pragma once

#include "Game/SimpleTrailedRocket.h"


namespace Hilltop {
namespace Game {

class GroundTrailedRocket : public SimpleTrailedRocket {
private:
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive &ar, const unsigned int version) {
        ar & boost::serialization::base_object<SimpleTrailedRocket>(*this);
    }

protected:
    GroundTrailedRocket();

public:
    static std::shared_ptr<GroundTrailedRocket> create();

    virtual void onTick(TankMatch *match) override;
    virtual void onHit(TankMatch *match) override;
};

}
}
