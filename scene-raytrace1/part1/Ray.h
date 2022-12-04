#pragma once
#include "Utils/toml11/toml.hpp"
#include "Common.h"

struct Ray {
    
    Ray(const Point& o = Point(0.f, 0.f, 0.f),
        const Vec& d = Vec(1.f, 0.f, 0.f)) : o(o), d(d) {}

    Point At(float t) const {
        return o + t * d;
    }

    Point o;
    Vec d;
};