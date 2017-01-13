#include "Game/TankController.h"


namespace Hilltop {
namespace Game {

TankController::TankController() {}

std::shared_ptr<TankController> TankController::create() {
    return std::shared_ptr<TankController>(new TankController());
}

void TankController::addWeapon(std::shared_ptr<Weapon> weapon, int amount) {
    for (int i = 0; i < weapons.size(); i++) {
        if (weapons[i].first == weapon) {
            weapons[i].second += amount;
            return;
        }
    }

    weapons.push_back(std::make_pair(weapon, amount));
}

void TankController::addRandomWeapon() {
    const int idx = rand() % TankMatch::weapons.size();
    addWeapon(TankMatch::weapons[idx], 1);
}

int TankController::getWeaponCount() {
    int ret = 0;
    for (const std::pair<std::shared_ptr<Weapon>, int> &p : weapons)
        ret += p.second;
    return ret;
}

bool TankController::applyAI(TankMatch *match, TankController &player) {
    if (player.botAttempts.empty()) {
        if (player.botTargetAngle != -1 || player.botTargetPower != -1) {
            if (player.botStepsDone < BOT_STEPS) {
                if (player.botLastStepTick + BOT_TICKS_BETWEEN_STEPS <= match->tickNumber) {
                    int angleDelta = player.botTargetAngle - player.tank->angle;
                    if (angleDelta < 2)
                        angleDelta++;
                    player.tank->angle += angleDelta / 2;

                    int powerDelta = player.botTargetPower - player.tank->power;
                    if (powerDelta < 2)
                        powerDelta++;
                    player.tank->power += powerDelta / 2;

                    player.botStepsDone++;
                    player.botLastStepTick = match->tickNumber;
                }
            } else {
                player.tank->angle = player.botTargetAngle;
                player.botTargetAngle = -1;

                player.tank->power = player.botTargetPower;
                player.botTargetPower = -1;

                return true;
            }
        } else {
            for (int mult = -1; mult <= 1; mult += 2) {
                for (int i = -10; i <= 10; i++) {
                    int angle = player.tank->angle;
                    int angleOff;
                    if (i >= -5 && i <= 5)
                        angleOff = i;
                    else
                        angleOff = i * 4;
                    angle = std::max(0, std::min(180, angle + angleOff * mult));

                    int power = player.tank->power;
                    if (i >= -5 && i <= 5)
                        power += i;
                    else
                        power += i * 3;
                    power = std::max(0, std::min(100, power));

                    std::shared_ptr<BotAttempt> attempt = BotAttempt::create();
                    attempt->angle = player.tank->angle;
                    attempt->power = power;
                    attempt->position = player.tank->getProjectileBase();
                    attempt->direction = Tank::calcTrajectory(player.tank->angle, power);
                    attempt->physicsSpeed = BotAttempt::enableDebug ? 1 : BOT_ATTEMPT_SPEED;
                    attempt->maxEntityAge = BOT_MAX_ATTEMPT_TIME *
                        (BotAttempt::enableDebug ? BOT_ATTEMPT_SPEED : 1);
                    player.botAttempts.push_back(attempt);
                    match->addEntity(*attempt);
                }
            }

            for (int i = 0; i < RANDOM_ATTEMPTS_BY_BOT_DIFFICULTY[player.botDifficulty]; i++) {
                int angle = scale(rand(), 0, RAND_MAX, 0, 180);
                int power = scale(rand(), 0, RAND_MAX, 0, 100);

                std::shared_ptr<BotAttempt> attempt = BotAttempt::create();
                attempt->angle = angle;
                attempt->power = power;
                attempt->position = player.tank->getBarrelBase() + Tank::getProjectileBase(angle);
                attempt->direction = Tank::calcTrajectory(angle, power);
                attempt->physicsSpeed = BotAttempt::enableDebug ? 1 : BOT_ATTEMPT_SPEED;
                attempt->maxEntityAge = BOT_MAX_ATTEMPT_TIME *
                    (BotAttempt::enableDebug ? BOT_ATTEMPT_SPEED : 1);
                player.botAttempts.push_back(attempt);
                match->addEntity(*attempt);
            }

            player.currentWeapon = rand() % player.weapons.size();

            player.botStepsDone = 0;
            player.botLastStepTick = match->tickNumber;

            if (!player.botTargetTank || !player.botTargetTank->alive) {
                player.botTargetTank.reset();
                std::vector<std::shared_ptr<TankController>> players = match->players;
                for (int i = 0; i < players.size(); i++) {
                    if (!players[i]->tank->alive || players[i]->team == player.team) {
                        players.erase(players.begin() + i);
                        i--;
                    }
                }
                if (!players.empty()) {
                    std::sort(players.begin(), players.end(),
                        [](std::shared_ptr<TankController> x, std::shared_ptr<TankController> y)->bool {
                        return x->tank->health / 10 < y->tank->health / 10;
                    });
                    int maxEq = 0;
                    for (int i = 1; i < players.size(); i++) {
                        if (players[i]->tank->health / 10 == players[0]->tank->health / 10)
                            maxEq = i;
                        else
                            break;
                    }
                    int idx = maxEq;
                    if (maxEq > 0)
                        idx = rand() % maxEq;
                    player.botTarget = players[idx]->tank->getBarrelBase();
                    player.botTargetTank = players[idx]->tank;
                }
            }

            if (player.botTargetTank) {
                player.botTarget = player.botTargetTank->getBarrelBase();
            } else {
                player.botTarget = {
                    (float)(rand() % match->height),
                    (float)(rand() % match->width)
                };
            }
        }
    } else {
        std::sort(player.botAttempts.begin(), player.botAttempts.end(),
            [player](std::shared_ptr<BotAttempt> x, std::shared_ptr<BotAttempt> y)->bool {
            Vector2 p1 = x->position.round();
            Vector2 p2 = y->position.round();
            if (p1 != p2)
                return distance(player.botTarget, p1) < distance(player.botTarget, p2);

            int a1 = x->angle;
            int a2 = y->angle;
            if (a1 != a2)
                return std::abs(player.tank->angle - a1) < std::abs(player.tank->angle - a2);

            return std::abs(player.tank->power - x->power) < std::abs(player.tank->power - y->power);
        });

        player.botAttempts[0]->color = Console::GREEN;
        for (int i = 1; i < player.botAttempts.size(); i++)
            player.botAttempts[i]->color = Console::RED;

        for (int i = 0; i < player.botAttempts.size(); i++)
            if (!player.botAttempts[i]->hasHit && !player.botAttempts[i]->hasExpired)
                return false;

        player.botTargetAngle = player.botAttempts[0]->angle;
        player.botTargetPower = player.botAttempts[0]->power;

        for (int i = 0; i < player.botAttempts.size(); i++)
            match->removeEntity(*player.botAttempts[i]);
        player.botAttempts.clear();
    }

    return false;
}

}
}
