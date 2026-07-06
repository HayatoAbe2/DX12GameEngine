#include "Circle.h"

bool Circle::CheckCollision(const Circle& other) {
    Vector2 diff = center - other.center;
    float distSq = Dot(diff, diff);
    float radiusSum = radius + other.radius;

    return distSq <= radiusSum * radiusSum;
}
