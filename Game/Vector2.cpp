#include "Game/Vector2.h"


namespace Hilltop {
namespace Game {

float scale(float value, float fromLow, float fromHigh, float toLow, float toHigh) {
    return (value - fromLow) * (toHigh - toLow) / (fromHigh - fromLow) + toLow;
}

float distance(const Vector2 from, const Vector2 to) {
    Vector2 v = to - from;
    return std::sqrt(v.X * v.X + v.Y * v.Y);
}

void foreachPixel(const Vector2 from, const Vector2 to, std::function<bool(Vector2)> handler) {
    Vector2 start = from.round();
    Vector2 end = to.round();
    int steps = std::max(1, (int)std::ceil(std::max(std::abs(start.X - end.X), std::abs(start.Y - end.Y))));
    Vector2 lastPixel = start;
    for (int i = 0; i <= steps; i++) {
        float pos = scale(i, 0, steps, 0, 1);
        Vector2 p = (start + (end - start) * pos).round();
        if (i == 0 || p != lastPixel) {
            lastPixel = p;
            if (handler(p))
                break;
        }
    }
}

}
}
