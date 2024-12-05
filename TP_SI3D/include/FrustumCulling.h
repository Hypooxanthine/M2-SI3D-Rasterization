#pragma once

#include <array>
#include <cstdint>

#include "mat.h"

struct Vec4
{
    float x, y, z, w;

    inline constexpr Vec4(float x, float y, float z, float w)
        : x(x), y(y), z(z), w(w)
    {}

    inline constexpr Vec4(const float* data)
        : x(data[0]), y(data[1]), z(data[2]), w(data[3])
    {}

    inline constexpr Vec4()
        : x(0.f), y(0.f), z(0.f), w(0.f)
    {}

    inline constexpr Vec4 operator+(const Vec4& r) const
    {
        return { x + r.x, y + r.y, z + r.z, w + r.w };
    }

    inline constexpr Vec4 operator-(const Vec4& r) const
    {
        return { x - r.x, y - r.y, z - r.z, w - r.w };
    }
};

struct Plane
{
    inline constexpr Plane(const Vec4& v)
        : a(v.x), b(v.y), c(v.z), d(v.w)
    {}

    inline constexpr Plane(float a, float b, float c, float d)
        : a(a), b(b), c(c), d(d)
    {}

    inline constexpr Plane()
        : a(0.f), b(0.f), c(0.f), d(0.f)
    {}

    bool pointInFront(const Point& p) const
    {
        return a * p.x + b * p.y + c * p.z + d > 0.f;
    }

    bool pointBehind(const Point& p) const
    {
        return a * p.x + b * p.y + c * p.z + d < 0.f;
    }

    float a, b, c, d;
};

struct Frustum
{
    std::array<Plane, 6> planes;

    inline constexpr Frustum(const Plane& left, const Plane& top, const Plane& right, const Plane& bottom, const Plane& near, const Plane& far)
        : planes{ left, top, right, bottom, near, far }
    {}
};

struct AABB
{
    Point first;
    Point second;

    inline bool containsPoint(const Point& p) const
    {
        return p.x > first.x && p.x < second.x
            && p.y > first.y && p.y < second.y
            && p.z > first.z && p.z < second.z;
    }
};

inline const Frustum ViewFrustum(const Transform& viewProj)
{
    const auto& m = viewProj.m;

    const Vec4& col0 = m[0];
    const Vec4& col1 = m[1];
    const Vec4& col2 = m[2];
    const Vec4& col3 = m[3];
    
    // Extraction des plans du frustum depuis la matrice VP
    const Plane left(col3 + col0);
    const Plane right(col3 - col0);
    const Plane bottom(col3 + col1);
    const Plane top(col3 - col1);
    const Plane near(col3 + col2);
    const Plane far(col3 - col2);

    return { left, top, right, bottom, near, far };
}

inline bool AabbCrossesFrustum(const AABB& aabb, const Frustum& frustum)
{
    const std::array<Point, 8> corners = {
        Point(aabb.first.x, aabb.first.y, aabb.first.z),
        Point(aabb.first.x, aabb.first.y, aabb.second.z),
        Point(aabb.first.x, aabb.second.y, aabb.first.z),
        Point(aabb.first.x, aabb.second.y, aabb.second.z),
        Point(aabb.second.x, aabb.first.y, aabb.first.z),
        Point(aabb.second.x, aabb.first.y, aabb.second.z),
        Point(aabb.second.x, aabb.second.y, aabb.first.z),
        Point(aabb.second.x, aabb.second.y, aabb.second.z)
    };

    for (const Plane& plane : frustum.planes)
    {
        // Si un aabb a tous ses points "à l'extérieur" d'un des plans
        // du frustum, alors ce plan du frustum sépare l'aabb du frustum,
        // donc l'aabb n'est pas dans le frustum
        if (plane.pointBehind(corners[0])
            && plane.pointBehind(corners[1])
            && plane.pointBehind(corners[2])
            && plane.pointBehind(corners[3])
            && plane.pointBehind(corners[4])
            && plane.pointBehind(corners[5])
            && plane.pointBehind(corners[6])
            && plane.pointBehind(corners[7]))
        {
            return false;
        }
    }

    // Pourrait être amélioré. En effet, il est possible qu'aucun
    // plan du frustum ne sépare l'aabb du frustum, mais qu'un des
    // plans 

    return true;
}