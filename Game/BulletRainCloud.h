#pragma once

#include "Game/Entity.h"


namespace Hilltop {
namespace Game {

class BulletRainCloud : public Entity {
private:
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive &ar, const unsigned int version) {
        ar & boost::serialization::base_object<Entity>(*this);
    }

protected:
    BulletRainCloud();

public:
    static const int BULLET_EVERY_TICKS = 1;
    static const int CLOUD_WIDTH = 16;
    static const int RAIN_TICKS = 32;
    static constexpr float RAIN_DAMAGE = 0.55f;

    static std::shared_ptr<BulletRainCloud> create();

    virtual void onTick(TankMatch *match) override;
    virtual void onDraw(TankMatch *match, Console::DoublePixelBufferedConsole &console) override;
    virtual void onHit(TankMatch *match) override;
};

}
}
