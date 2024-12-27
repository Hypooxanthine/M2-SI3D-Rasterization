#pragma once

#include <array>
#include <cstdint>

#include "mat.h"

struct AABB
{
    Point first;
    Point second;
};

/**
 * Retourne vrai si un des plans du AABB permet de séparer tous les points "points" de tous les points
 * du AABB.
 */
inline bool PlaneBetweenBasedOnAabb(const AABB& aabb, const std::vector<Point>& points)
{
    bool control = true;
    // Première face : points.x tous < aabb.first.x

    for (const Point& p : points)
    {
        if (p.x > aabb.first.x)
        {
            control = false;
            break;
        }
    }
    
    if (control)
        return true;

    // Deuxième face : points.x tous > aabb.second.x
    control = true;

    for (const Point& p : points)
    {
        if (p.x < aabb.second.x)
        {
            control = false;
            break;
        }
    }

    if (control)
        return true;

    // Troisième face : points.y tous < aabb.first.y

    control = true;

    for (const Point& p : points)
    {
        if (p.y > aabb.first.y)
        {
            control = false;
            break;
        }
    }

    if (control)
        return true;

    // Quatrième face : points.y tous > aabb.second.y

    control = true;

    for (const Point& p : points)
    {
        if (p.y < aabb.second.y)
        {
            control = false;
            break;
        }
    }

    if (control)
        return true;

    // Cinquième face : points.z tous < aabb.first.z

    control = true;

    for (const Point& p : points)
    {
        if (p.z > aabb.first.z)
        {
            control = false;
            break;
        }
    }

    if (control)
        return true;

    // Sixième face : points.z tous > aabb.second.z

    control = true;

    for (const Point& p : points)
    {
        if (p.z < aabb.second.z)
        {
            control = false;
            break;
        }
    }

    return control;
}

inline const std::vector<Point> ndc_points = {
    Point(-1, -1, -1),
    Point(-1, -1,  1),
    Point(-1,  1, -1),
    Point(-1,  1,  1),
    Point( 1, -1, -1),
    Point( 1, -1,  1),
    Point( 1,  1, -1),
    Point( 1,  1,  1)
};

inline const AABB ndc_aabb = {
    Point(-1, -1, -1),
    Point( 1,  1,  1)
};

inline std::vector<Point> extractFrustumPoints(const Transform& viewProjInv)
{
    auto out = ndc_points;

    for (auto& p : out)
        p = viewProjInv(p);
    
    return out;
}

inline std::vector<Point> getAabbPoints(const AABB& aabb)
{
    return {
        Point(aabb.first.x, aabb.first.y, aabb.first.z),
        Point(aabb.first.x, aabb.first.y, aabb.second.z),
        Point(aabb.first.x, aabb.second.y, aabb.first.z),
        Point(aabb.first.x, aabb.second.y, aabb.second.z),
        Point(aabb.second.x, aabb.first.y, aabb.first.z),
        Point(aabb.second.x, aabb.first.y, aabb.second.z),
        Point(aabb.second.x, aabb.second.y, aabb.first.z),
        Point(aabb.second.x, aabb.second.y, aabb.second.z)
    };
}

inline bool AabbCrossesFrustum(const AABB& aabb, const Transform& viewProj)
{
    if (PlaneBetweenBasedOnAabb(aabb, extractFrustumPoints(Inverse(viewProj))))
        return false;

    auto aabb_points = getAabbPoints(aabb);
    for (Point& p : aabb_points)
    {
        p = viewProj(p);
    }

    if (PlaneBetweenBasedOnAabb(ndc_aabb, aabb_points))
        return false;

    return true;
}