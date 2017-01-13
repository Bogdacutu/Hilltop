#include "Game/Tank.h"
#include "Game/SimpleRocket.h"
#include "Game/TankMatch.h"
#include "Game/TankWheel.h"


namespace Hilltop {
namespace Game {

Tank::Tank(Console::ConsoleColor color) : Entity(), color(color) {
    gravityMult = 0.0f;
}

Console::ConsoleColor Tank::getActualColor() {
    return alive ? color : DEAD_COLOR;
}

Vector2 Tank::getBarrelBase() {
    return position.round() + Vector2(-2.0f, 2.0f);
}

Vector2 Tank::getBarrelEnd() {
    return getBarrelBase() + getBarrelEnd(angle);
}

Vector2 Tank::getBarrelEnd(int angle) {
    static const float DS2 = std::sqrtf(2.0) * 2.0f;

    float a = angle * PI / 180.0f;

    Vector2 c = { -std::sinf(a), std::cosf(a) };
    Vector2 c2 = { c.X * c.X, c.Y * c.Y };
    Vector2 subterm = { 2.0f + c2.X - c2.Y, 2.0f - c2.X + c2.Y };
    Vector2 term1 = { subterm.X + c.X * DS2, subterm.Y + c.Y * DS2 };
    Vector2 term2 = { subterm.X - c.X * DS2, subterm.Y - c.Y * DS2 };
    Vector2 r = Vector2(std::sqrtf(term1.X) - std::sqrtf(term2.X),
        std::sqrtf(term1.Y) - std::sqrtf(term2.Y)) * 0.5f;

    return r * 2.0f;
}

Vector2 Tank::getProjectileBase() {
    return getBarrelBase() + getProjectileBase(angle);
}

Vector2 Tank::getProjectileBase(Vector2 barrelEnd) {
    Vector2 p = barrelEnd.round();
    Vector2 r = p;

    if (p.X >= 1.0f)
        r.X += 1.0f;
    else if (p.X <= -1.0f)
        r.X -= 1.0f;

    if (p.Y >= 1.0f)
        r.Y += 1.0f;
    else if (p.Y <= -1.0f)
        r.Y -= 1.0f;

    return r;
}

Vector2 Tank::getProjectileBase(int angle) {
    return getProjectileBase(getBarrelEnd(angle));
}

Vector2 Tank::calcTrajectory(int angle, int power) {
    float ang = (float)angle * PI / 180.0f;
    return Vector2(-std::sinf(ang), std::cosf(ang)) * 8.0f * ((float)power / 100.0f);
}

Vector2 Tank::calcTrajectory() {
    return Tank::calcTrajectory(angle, power);
}

std::vector<Vector2> Tank::getPixels() {
    std::vector<Vector2> pixels;

    // the barrel
    float a = angle * PI / 180.0f;
    foreachPixel(getBarrelBase(), getBarrelEnd(), [&pixels](Vector2 v)->bool {
        pixels.push_back(v);
        return false;
    });

    // the rest of the tank
    Vector2 p = position.round();
    for (int i = 0; i < 5; i++)
        pixels.push_back(p + Vector2(-1, i));
    for (int i = 1; i < 4; i++)
        pixels.push_back(p + Vector2(-2, i));

    return pixels;
}

bool Tank::testCollision(Vector2 position) {
    position = position.round();
    int x = position.X;
    int y = position.Y;

    for (Vector2 &v : getPixels()) {
        int dx = v.X;
        int dy = v.Y;
        if (dx == x && dy == y)
            return true;
    }

    return false;
}

std::shared_ptr<Tank> Tank::create(Console::ConsoleColor color) {
    return std::shared_ptr<Tank>(new Tank(color));
}

void Tank::initWheels(TankMatch &match) {
    for (int i = 0; i < 5; i++) {
        if (!wheels[i])
            wheels[i] = TankWheel::create();
        wheels[i]->position = { position.X, position.Y + i };
        match.addEntity(*wheels[i]);
    }
}

void Tank::onTick(TankMatch *match) {
    Entity::onTick(match);

    float top = wheels[0]->position.X;
    Vector2 leastDir = wheels[0]->direction;
    for (int i = 1; i < 5; i++) {
        top = std::min(top, wheels[i]->position.X);
        if (wheels[i]->direction.X < leastDir.X)
            leastDir = wheels[i]->direction;
    }
    position.X = top;

    for (int i = 0; i < 5; i++) {
        wheels[i]->position = { position.X, position.Y + i };
        wheels[i]->direction = leastDir;
    }
}

void Tank::onDraw(TankMatch *match, Console::DoublePixelBufferedConsole &console) {
    const static float pi = std::atanf(1) * 4;

    Entity::onDraw(match, console);

    Console::ConsoleColor c = getActualColor();

    for (Vector2 &v : getPixels())
        console.set(v.X, v.Y, c);

    Vector2 p = position.round();

    int hpTop = p.X + 2;
    if (hpTop >= match->height - 1)
        hpTop = p.X - 8;

    if (alive) {
        int hp = scale(health, 0, maxHealth, 0, 8);
        for (int i = 0; i < 9; i++)
            console.set(hpTop, p.Y - 2 + i, Console::RED);
        for (int i = 0; i <= hp; i++)
            console.set(hpTop, p.Y - 2 + i, Console::GREEN);

        if (armor > 0) {
            int ap = scale(armor, 0, maxArmor, 0, 8);
            for (int i = 0; i <= ap; i++)
                console.set(hpTop + 1, p.Y - 2 + i, Console::BLUE);
        }
    }
}

void Tank::drawReticle(TankMatch *match, Console::DoublePixelBufferedConsole &console) {
    float ang = (float)angle * PI / 180.0f;
    Vector2 p = (getBarrelBase().round() + Vector2(-std::sinf(ang), std::cosf(ang)) * 10.0f).round();
    console.set(p.X, p.Y, Console::WHITE);
}

bool Tank::canMove(TankMatch *match, int direction) {
    Vector2 p = position;
    doMove(match, direction);
    if (p != position) {
        position = p;
        return true;
    } else {
        return false;
    }
}

void Tank::doMove(TankMatch *match, int direction) {
    Vector2 p = position.round() + Vector2(0, direction);
    if (p.X < 0.0f || p.X >= match->width - 5.0f)
        return;

    bool ret;
    for (int off = 0; off <= 3; off++) {
        ret = true;
        for (int i = 1; i <= 2; i++) {
            for (int j = 0; j < 5; j++) {
                if (match->get(p.X - i, p.Y + j) != TankMatch::AIR) {
                    ret = false;
                    break;
                }
            }
        }
        if (ret) {
            position = p;
            break;
        }
        p.X--;
    }

    if (ret)
        initWheels(*match);
}

void Tank::dealDamage(TankMatch *match, int damage) {
    if (armor > 0) {
        armor = std::max(0, armor - damage);
    } else {
        health = std::max(0, health - damage);
        if (health == 0)
            die(match);
    }
}

void Tank::die(TankMatch *match) {
    if (!alive)
        return;
    alive = false;

    if (match) {
        int angle = 75;
        for (int i = 0; i < 2; i++) {
            std::shared_ptr<SimpleRocket> rocket = SimpleRocket::create(Console::WHITE);
            rocket->position = position.round() + Vector2(-2, 2);
            rocket->direction = calcTrajectory(angle, 10);
            rocket->explosionSize = 4;
            rocket->destroyLand = false;
            match->addEntity(*rocket);

            angle += 30;
        }
    }
}

}
}
