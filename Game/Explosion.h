#pragma once

#include "Game/Entity.h"
#include "Game/Tank.h"
#include <set>


namespace Hilltop {
namespace Game {

class Explosion : public Entity {
private:
    friend class boost::serialization::access;
    template<class Archive>
    friend inline void load_construct_data(Archive &ar, Explosion *t, const unsigned int);
    template<class Archive>
    void serialize(Archive &ar, const unsigned int version) {
        ar & boost::serialization::base_object<Entity>(*this);
        ar & coreSize;
        ar & damageMult;
        ar & tanksHit;
    }

protected:
    Explosion(int size);

    static const int ticksBetween = 2;

    void destroyLand(TankMatch *match);
    void createLand(TankMatch *match);

public:
    const int size;
    int coreSize;
    float damageMult = 1.0f;

    bool willDestroyLand = false;
    bool willCreateLand = false;

    std::set<std::shared_ptr<Tank>> tanksHit;
    int calcDamage(Vector2 point);
    void hitTanks(TankMatch *match);

    static std::shared_ptr<Explosion> create(int size);

    virtual void onTick(TankMatch *match) override;
    virtual void onDraw(TankMatch *match, Console::DoublePixelBufferedConsole &console) override;
};

template<class Archive>
inline void save_construct_data(Archive &ar, const Explosion *t, const unsigned int) {
    ar << t->size;
}

template<class Archive>
void load_construct_data(Archive &ar, Explosion *t, const unsigned int) {
    int size;
    ar >> size;
    ::new(t) Explosion(size);
}

}
}
