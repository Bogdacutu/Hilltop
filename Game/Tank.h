#pragma once

#include "Game/Entity.h"


namespace Hilltop {
namespace Game {

class Tank : public Entity {
private:
    friend class boost::serialization::access;
    template<class Archive>
    friend inline void load_construct_data(Archive &ar, Tank *t, const unsigned int);
    template<class Archive>
    void serialize(Archive &ar, const unsigned int version) {
        ar & boost::serialization::base_object<Entity>(*this);
        ar & color;
        ar & wheels;
        ar & alive;
        ar & angle;
        ar & power;
        ar & maxHealth;
        ar & health;
        ar & maxArmor;
        ar & armor;
        ar & damage;
    }

protected:
    Tank(Console::ConsoleColor color);

public:
    static const Console::ConsoleColor DEAD_COLOR = Console::DARK_GRAY;

    Console::ConsoleColor color;
    Console::ConsoleColor getActualColor();

    std::shared_ptr<Entity> wheels[5];

    bool alive = true;
    int angle = 45;
    int power = 50;
    int maxHealth = 100;
    int health = 100;
    int maxArmor = 0;
    int armor = 0;
    float damage = 1.0f;

    Vector2 getBarrelBase();
    Vector2 getBarrelEnd();
    static Vector2 getBarrelEnd(int angle);
    Vector2 getProjectileBase();
    static Vector2 getProjectileBase(Vector2 barrelEnd);
    static Vector2 getProjectileBase(int angle);

    static Vector2 calcTrajectory(int angle, int power);
    Vector2 calcTrajectory();
    std::vector<Vector2> getPixels();
    bool testCollision(Vector2 position);

    static std::shared_ptr<Tank> create(Console::ConsoleColor color);
    void initWheels(TankMatch &match);

    virtual void onTick(TankMatch *match) override;
    virtual void onDraw(TankMatch *match, Console::DoublePixelBufferedConsole &console) override;

    void drawReticle(TankMatch *match, Console::DoublePixelBufferedConsole &console);

    bool canMove(TankMatch *match, int direction);
    void doMove(TankMatch *match, int direction);

    void dealDamage(TankMatch *match, int damage);
    void die(TankMatch *match);
};

template<class Archive>
inline void load_construct_data(Archive &ar, Tank *t, const unsigned int) {
    ::new(t) Tank(ConsoleColor());
}

}
}
