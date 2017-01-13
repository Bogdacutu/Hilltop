#pragma once

#include "Game/BotAttempt.h"
#include "Game/Tank.h"
#include "Game/TankMatch.h"
#include "Game/Weapon.h"


namespace Hilltop {
namespace Game {

class TankController : public std::enable_shared_from_this<TankController> {
private:
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive &ar, const unsigned int version) {
        ar & botAttempts;
        ar & botTargetTank;
        ar & botTarget;
        ar & botTargetAngle;
        ar & botTargetPower;
        ar & botStepsDone;
        ar & botDifficulty;
        ar & botLastStepTick;
        ar & tank;
        ar & team;
        ar & isHuman;
        ar & currentWeapon;
        ar & movesPerTurn;
        ar & movesLeft;
        ar & weapons;
    }

protected:
    TankController();

public:
    static const int BOT_ATTEMPT_SPEED = 5;
    static const int BOT_MAX_ATTEMPT_TIME = 80;
    static constexpr int RANDOM_ATTEMPTS_BY_BOT_DIFFICULTY[] = {
        5, 20, 80
    };
    static const int BOT_STEPS = 6;
    static const int BOT_TICKS_BETWEEN_STEPS = 4;
    std::vector<std::shared_ptr<BotAttempt>> botAttempts;
    std::shared_ptr<Tank> botTargetTank;
    Vector2 botTarget;
    int botTargetAngle = -1;
    int botTargetPower = -1;
    int botStepsDone;
    int botLastStepTick;
    int botDifficulty;

    std::shared_ptr<Tank> tank;
    int team = 1;
    bool isHuman = true;
    int currentWeapon = 0;
    int movesPerTurn = 25;
    int movesLeft = movesPerTurn;

    static std::shared_ptr<TankController> create();

    std::vector<std::pair<std::shared_ptr<Weapon>, int>> weapons;
    void addWeapon(std::shared_ptr<Weapon> weapon, int amount);
    void addRandomWeapon();
    int getWeaponCount();

    static bool applyAI(TankMatch *match, TankController &player);
};

}
}
