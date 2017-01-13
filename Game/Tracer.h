#pragma once

#include "Game/Entity.h"


namespace Hilltop {
namespace Game {

class Tracer : public Entity {
private:
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive &ar, const unsigned int version) {
        ar & boost::serialization::base_object<Entity>(*this);
        ar & text;
    }

protected:
    Tracer();

public:
    static const int DURATION = 30;

    std::string text;

    static std::shared_ptr<Tracer> create();

    virtual void onTick(TankMatch *match) override;
    virtual void onDraw(TankMatch *match, Console::DoublePixelBufferedConsole &console) override;
    virtual void onDirectDraw(TankMatch *match, Console::BufferedConsole &console);
    virtual void onHit(TankMatch *match) override;
};

}
}
