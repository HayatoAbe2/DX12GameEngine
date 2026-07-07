#pragma once
#include "Segment.h"

struct Capsule2D {
    Segment2D segment;
    float radius;
};

struct Capsule3D {
    Segment3D segment;
    float radius;
};