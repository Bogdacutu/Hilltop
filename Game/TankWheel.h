#pragma once

#include "Game/Entity.h"


namespace Hilltop {
namespace Game {

class TankWheel : public Entity {
private:
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive &ar, const unsigned int version) {
        ar & boost::serialization::base_object<Entity>(*this);
    }

protected:
    TankWheel();

public:
    static bool enableDebug;

    static std::shared_ptr<TankWheel> create();

    virtual void onDraw(TankMatch *match, Console::DoublePixelBufferedConsole &console) override;
};

}
}
