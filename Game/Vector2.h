#pragma once

#include <algorithm>
#include <boost/serialization/access.hpp>
#include <functional>

#pragma warning(disable: 4244)


namespace Hilltop {
namespace Game {

const static float PI = std::atanf(1) * 4;

float scale(float value, float fromLow, float fromHigh, float toLow, float toHigh);

class Vector2 {
private:
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive &ar, const unsigned int version) {
        ar & X;
        ar & Y;
    }

public:
    float X = 0;
    float Y = 0;

    Vector2() {}
    Vector2(float X, float Y) : X(X), Y(Y) {}

    Vector2 operator+(const Vector2 &other) const {
        return Vector2(X + other.X, Y + other.Y);
    }

    Vector2 operator-(const Vector2 &other) const {
        return Vector2(X - other.X, Y - other.Y);
    }

    Vector2 operator-() const {
        return Vector2(-X, -Y);
    }

    Vector2 operator*(float other) const {
        return Vector2(X * other, Y * other);
    }

    bool operator==(const Vector2 &other) const {
        return X == other.X && Y == other.Y;
    }

    bool operator!=(const Vector2 &other) const {
        return X != other.X || Y != other.Y;
    }

    Vector2 floor() const {
        return Vector2(std::floor(X), std::floor(Y));
    }

    Vector2 round() const {
        return Vector2(std::round(X), std::round(Y));
    }

    Vector2 ceil() const {
        return Vector2(std::ceil(X), std::ceil(Y));
    }

    Vector2 abs() const {
        return Vector2(std::abs(X), std::abs(Y));
    }
};

float distance(const Vector2 from, const Vector2 to);

void foreachPixel(const Vector2 from, const Vector2 to, std::function<bool(Vector2)> handler);

}
}
