#pragma once

#include "Game/Entity.h"
#include "Game/Tank.h"


namespace Hilltop {
namespace Game {

class Drop : public Entity {
private:
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive &ar, const unsigned int version) {
        ar & boost::serialization::base_object<Entity>(*this);
        ar & color;
        ar & ch;
    }

protected:
    Drop();

public:
    Console::ConsoleColor color = Console::BLACK;
    wchar_t ch = L' ';

    int getTopRow();

    virtual void onTick(TankMatch *match) override;
    virtual void onDirectDraw(TankMatch *match, Console::BufferedConsole &console) override;

    virtual void handleTank(TankMatch *match, Tank &tank);
};

}
}
