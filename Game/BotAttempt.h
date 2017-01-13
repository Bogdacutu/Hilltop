#pragma once

#include "Game/Entity.h"


namespace Hilltop {
namespace Game {

class BotAttempt : public Entity {
private:
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive &ar, const unsigned int version) {
        ar & boost::serialization::base_object<Entity>(*this);
        ar & color;
        ar & angle;
        ar & power;
    }

protected:
    BotAttempt();

public:
    static bool enableDebug;
    Console::ConsoleColor color = Console::RED;

    int angle;
    int power;

    static std::shared_ptr<BotAttempt> create();

    virtual void onDraw(TankMatch *match, Console::DoublePixelBufferedConsole &console) override;
};

}
}
