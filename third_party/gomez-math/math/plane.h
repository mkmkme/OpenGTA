#ifndef MATH_PLANE_H
#define MATH_PLANE_H

#include <glm/ext/vector_float3.hpp>
#include <glm/geometric.hpp>

namespace Math {

float intersection_segments(const glm::vec3 &s1, const glm::vec3 &s2, const glm::vec3 &l1, const glm::vec3 &l2, glm::vec3 &hit);

struct Plane {
    Plane(const glm::vec3 &p, const glm::vec3 &n)
        : pop(p), normal(n) {}
    Plane(const glm::vec3 &p1, const glm::vec3 &p2, const glm::vec3 &p3)
        : pop(p1), normal()
    {
        const glm::vec3 pa(p2 - p1);
        const glm::vec3 pb(p3 - p1);
        normal = glm::normalize(glm::cross(pa, pb));
    }
    glm::vec3 pop;
    glm::vec3 normal;
    int segmentIntersect(const glm::vec3 &p1, const glm::vec3 &p2, glm::vec3 &p_p) const;
    [[nodiscard]] float distance(const glm::vec3 &p) const;
};
} // namespace Math
#endif
