#pragma once

#include "Game/Entity.h"
#include <boost/serialization/access.hpp>
#include <memory>
#include <string>


namespace Hilltop {
namespace Game {

class TankMatch;

class Weapon {
private:
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive &ar, const unsigned int version) {
        ar & name;
    }

public:
    static const std::string INVALID_NAME;

    std::string name = INVALID_NAME;

    virtual void fire(TankMatch &match, int playerNumber);
};

}
}
