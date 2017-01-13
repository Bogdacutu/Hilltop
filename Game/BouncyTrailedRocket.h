#pragma once

#include "Game/SimpleTrailedRocket.h"


namespace Hilltop {
namespace Game {

class BouncyTrailedRocket : public SimpleTrailedRocket {
private:
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive &ar, const unsigned int version) {
        ar & boost::serialization::base_object<SimpleTrailedRocket>(*this);
        ar & bouncesLeft;
    }

protected:
    BouncyTrailedRocket();

public:
    static const int MAX_BOUNCES = 4;

    int bouncesLeft = MAX_BOUNCES;

    static std::shared_ptr<BouncyTrailedRocket> create();

    virtual void onHit(TankMatch *match) override;

    void finish(TankMatch *match);
};

}
}
