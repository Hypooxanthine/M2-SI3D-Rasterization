#pragma once

#include <array>
#include <cstdint>

#include "mat.h"

struct AABB
{
    Point first;
    Vector size;

    inline bool containsPoint(const Point& p) const
    {
        const Point second = first + size;

        return p.x > first.x && p.x < second.x
            && p.y > first.y && p.y < second.y
            && p.z > first.z && p.z < second.z;
    }
};

inline const AABB NDC_CUBE = {
    .first = { -1.f, -1.f, -1.f},
    .size = { 2.f, 2.f, 2.f }
};

struct Octogon
{
    inline Octogon() = default;

    inline constexpr Octogon(const Octogon& other)
        : points(other.points)
    {}

    std::array<Point, 8> points;

    using iterator = decltype(points)::iterator;
    using const_iterator = decltype(points)::const_iterator;

    inline constexpr const Point& operator[](size_t i) const { return points.at(i); };
    inline constexpr Point& operator[](size_t i) { return points.at(i); }

    iterator begin() { return points.begin(); }
    iterator end() { return points.end(); }

    const_iterator begin() const { return points.begin(); }
    const_iterator end() const { return points.end(); }
};

inline Octogon AabbToOctogon(const AABB& aabb)
{
    Octogon out;

    const auto& first = aabb.first;
    const auto& size = aabb.size;

    for (uint8_t i = 0; i < 4; ++i)
        out[i] = first;
    
    out[1].y += size.y;
    out[2].y += size.y;
    out[2].x += size.x;
    out[3].x += size.x;

    for (uint8_t i = 4; i < 8; ++i)
    {
        out[i] = out[i - 4];
        out[i].z += size.z;
    }

    return out;
}

inline const Octogon NDC_OCTOGON = AabbToOctogon(NDC_CUBE);

inline bool AabbCrossesViewVolume(const AABB& aabb_ws, const Transform& viewProj, const Transform& viewProjInv)
{
    // Premier passage : aabb contre frustum en espace monde

    auto frustum_ws = NDC_OCTOGON;
    for (auto& p : frustum_ws)
        p = viewProjInv(p);

    for (const auto& p : frustum_ws)
        if (aabb_ws.containsPoint(p))
            return true;
    
    // Deuxième passage : aabb contre frustum en espace projectif

    auto aabb_ndc = AabbToOctogon(aabb_ws);
    for (auto& p : aabb_ndc)
        p = viewProj(p);

    for (const auto& p : aabb_ndc)
        if (NDC_CUBE.containsPoint(p))
            return true;

    return false;
}
