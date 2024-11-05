#include "plane.h"

#include <cmath>

#include <glm/geometric.hpp>

namespace Math {

// see: http://local.wasp.uwa.edu.au/~pbourke/geometry/lineline2d/example.cpp
float intersection_segments(const glm::vec3 &s1, const glm::vec3 &s2, const glm::vec3 &l1, const glm::vec3 &l2, glm::vec3 &hit)
{
    float denom = (l2.z - l1.z) * (s2.x - s1.x) -
        (l2.x - l1.x) * (s2.z - s1.z);

    float d_a = (l2.x - l1.x) * (s1.z - l1.z) -
        (l2.z - l1.z) * (s1.x - l1.x);

    float d_b = (s2.x - s1.x) * (s1.z - l1.z) -
        (s2.z - s1.z) * (s1.x - l1.x);

    float ua = d_a / denom; // ratio on s1 -> s2
    float ub = d_b / denom; // ratio on l1 -> l2

    if (denom == 0.0f) {
        return -1.0f;
    }

    if (ua >= 0.0f && ua <= 1.0f && ub >= 0.0f && ub <= 1.0f) {
        hit.x = s1.x + ua * (s2.x - s1.x);
        hit.z = s1.z + ua * (s2.z - s1.z);
        return ub;
    }
    return -1.0f;
}

int Plane::segmentIntersect(const glm::vec3 &p1, const glm::vec3 &p2, glm::vec3 &p_p) const
{
    float d = -normal.x * pop.x - normal.y * pop.y - normal.z * pop.z;
    float denom = normal.x * (p2.x - p1.x) + normal.y * (p2.y - p1.y) +
        normal.z * (p2.z - p1.z);
    if (std::fabs(denom) < 0.001f)
        return false;
    float mu = -(d + normal.x * p1.x + normal.y * p1.y + normal.z * p1.z) / denom;
    p_p = (p2 - p1) * mu + p1;
    return mu >= 0 && mu <= 1;
}
float Plane::distance(const glm::vec3 &p) const
{
    float d = glm::dot(normal, p);
    d += -normal.x * pop.x - normal.y * pop.y - normal.z * pop.z;
    return d;
}
} // namespace Math
