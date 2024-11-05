/* Derived from code written by Jonathan Kreuzer.
 *
 * See: http://www.3dkingdoms.com/weekly/weekly.php?a=21
 *
 * basically the same as bbox.h/.cpp but using coldet math
 *
 * -- quote from a mail of the author --
 *
 * You're free to continue using my CBBox code however you want.
 * ... [snip] ...
 * The only thing I ask is a note about where it came from ( I think
 * you said you added a link to the article, so that's fine. )
 *
 */

/************************************************************************
 * Copyright (c) 2005-2007 tok@openlinux.org.uk                          *
 *                                                                       *
 * This software is provided as-is, without any express or implied       *
 * warranty. In no event will the authors be held liable for any         *
 * damages arising from the use of this software.                        *
 *                                                                       *
 * Permission is granted to anyone to use this software for any purpose, *
 * including commercial applications, and to alter it and redistribute   *
 * it freely, subject to the following restrictions:                     *
 *                                                                       *
 * 1. The origin of this software must not be misrepresented; you must   *
 * not claim that you wrote the original software. If you use this       *
 * software in a product, an acknowledgment in the product documentation *
 * would be appreciated but is not required.                             *
 *                                                                       *
 * 2. Altered source versions must be plainly marked as such, and must   *
 * not be misrepresented as being the original software.                 *
 *                                                                       *
 * 3. This notice may not be removed or altered from any source          *
 * distribution.                                                         *
 ************************************************************************/

#include "obox.h"

#include <cassert>

#include <glm/common.hpp>
#include <glm/geometric.hpp>
#include <glm/matrix.hpp>

#include "plane.h"
// --------------------------
//
// Oriented Bounding Box Class
//
// --------------------------

//
// Check if a point is in this bounding box
//
bool OBox::isPointInBox(const glm::vec3 &InP) const
{
    // Rotate the point into the box's coordinates
    const auto P = transformCoords(InP);
    // Now just use an axis-aligned check
    return glm::all(glm::lessThan(glm::abs(P), extent_));
}

//
// Check if a sphere overlaps any part of this bounding box
//
bool OBox::isSphereInBox(const glm::vec3 &InP, float fRadius) const
{
    float fDist;
    float fDistSq = 0;
    auto P = transformCoords(InP);

    // Add distance squared from sphere centerpoint to box for each axis
    for (int i = 0; i < 3; i++) {
        if (fabs(P[i]) > extent_[i]) {
            fDist = fabs(P[i]) - extent_[i];
            fDistSq += fDist * fDist;
        }
    }
    return (fDistSq <= fRadius * fRadius);
}

//
// Check if the bounding box is completely behind a plane( defined by a normal and a point )
//
bool OBox::boxOutsidePlane(const glm::vec3 &norm, const glm::vec3 &p) const
{
    // Plane Normal in Box Space

    const auto Norm = glm::abs(glm::inverse(glm::mat3(transform_)) * norm);

    const auto Extent = glm::dot(Norm, extent_); // Norm.Dot( m_Extent ); // Box Extent along the plane normal
    // float Distance = InNorm.Dot( GetCenterPoint() - InP ); // Distance from Box Center to the Plane
    const auto Distance = glm::dot(norm, (getCenterPoint() - p));
    // If Box Centerpoint is behind the plane further than its extent, the Box is outside the plane
    return Distance < -Extent;
}

//
// Does the Line (L1, L2) intersect the Box?
//
bool OBox::isLineInBox(const glm::vec3 &L1, const glm::vec3 &L2) const
{
    // Put line in box space
    const auto MInv = glm::inverse(transform_);
    const auto LB1 = glm::vec3(MInv * glm::vec4(L1, 1.0f));
    const auto LB2 = glm::vec3(MInv * glm::vec4(L2, 1.0f));

    // Get line midpoint and extent
    const auto LMid = (LB1 + LB2) * 0.5f;
    const auto L = (LB1 - LMid);
    const auto LExt = glm::abs(L);

    // Use Separating Axis Test
    // Separation vector from box center to line center is LMid, since the line is in box space
    if (fabs(LMid.x) > extent_.x + LExt.x)
        return false;
    if (fabs(LMid.y) > extent_.y + LExt.y)
        return false;
    if (fabs(LMid.z) > extent_.z + LExt.z)
        return false;
    // Crossproducts of line and each axis
    if (fabs(LMid.y * L.z - LMid.z * L.y) > (extent_.y * LExt.z + extent_.z * LExt.y))
        return false;
    if (fabs(LMid.x * L.z - LMid.z * L.x) > (extent_.x * LExt.z + extent_.z * LExt.x))
        return false;
    if (fabs(LMid.x * L.y - LMid.y * L.x) > (extent_.x * LExt.y + extent_.y * LExt.x))
        return false;
    // No separating axis, the line intersects
    return true;
}

void OBox::lineCrossBox(const glm::vec3 &L1, const glm::vec3 &L2, glm::vec3 &isecLocal) const
{
    // Put line in box space
    const auto MInv = glm::inverse(transform_);
    const auto LB1 = glm::vec3(MInv * glm::vec4(L1, 1.0f));
    const auto LB2 = glm::vec3(MInv * glm::vec4(L2, 1.0f));
    float small_t = 2.0f;
    glm::vec3 p_copy;

    // i = 0: -x,-z <-> -x,z
    // i = 1: -x,-z <-> x,-z
    // i = 2: x,-z, <-> x,z
    // i = 3: -x,z  <-> x,z
    for (int i = 0; i < 4; i++) {
        glm::vec3 s1((i <= 1 || i == 3 ? -extent_.x : extent_.x), 0, (i < 3 ? -extent_.z : extent_.z));
        glm::vec3 s2((i == 0 ? -extent_.x : extent_.x), 0, (i == 1 ? -extent_.z : extent_.z));
        glm::vec3 p;
        float dt = Math::intersection_segments(s1, s2, LB1, LB2, p);
        if ((dt >= 0.0f) && (dt < small_t)) {
            p_copy = p;
            small_t = dt;
        }
    }
    if (small_t >= 0.0f && small_t <= 1.0f) {
        isecLocal = p_copy;
        return;
    }
    // ERROR("Did not find intersection when OBB says there is one :-(");
    isecLocal = L1;
}

//
// Returns a 3x3 rotation matrix as vectors
//
inline std::array<glm::vec3, 3> OBox::getInvRot() const
{
    return {
        glm::vec3(transform_[0][0], transform_[0][1], transform_[0][2]),
        glm::vec3(transform_[1][0], transform_[1][1], transform_[1][2]),
        glm::vec3(transform_[2][0], transform_[2][1], transform_[2][2])
    };
}

//
// Check if any part of a box is inside any part of another box
// Uses the separating axis test.
//
bool OBox::isBoxInBox(OBox &BBox) const
{
    const auto SizeA = extent_;
    const auto SizeB = BBox.extent_;
    const auto RotA = getInvRot();
    const auto RotB = BBox.getInvRot();

    glm::mat3 R;  // Rotation from B to A
    glm::mat3 AR; // absolute values of R matrix, to use with box extents
    float ExtentA, ExtentB, Separation;
    int i, k;

    // Calculate B to A rotation matrix
    for (i = 0; i < 3; i++)
        for (k = 0; k < 3; k++) {
            R[i][k] = glm::dot(RotA[i], RotB[k]);
            AR[i][k] = fabs(R[i][k]);
        }

    // Vector separating the centers of Box B and of Box A
    const auto vSepWS = BBox.getCenterPoint() - getCenterPoint();
    // Rotated into Box A's coordinates
    glm::vec3 vSepA(glm::dot(vSepWS, RotA[0]), glm::dot(vSepWS, RotA[1]), glm::dot(vSepWS, RotA[2]));

    // Test if any of A's basis vectors separate the box
    for (i = 0; i < 3; i++) {
        ExtentA = SizeA[i];
        ExtentB = glm::dot(SizeB, glm::vec3(AR[i][0], AR[i][1], AR[i][2]));
        Separation = fabs(vSepA[i]);

        if (Separation > ExtentA + ExtentB)
            return false;
    }

    // Test if any of B's basis vectors separate the box
    for (k = 0; k < 3; k++) {
        ExtentA = glm::dot(SizeA, glm::vec3(AR[0][k], AR[1][k], AR[2][k]));
        ExtentB = SizeB[k];
        Separation = fabs(glm::dot(vSepA, glm::vec3(R[0][k], R[1][k], R[2][k])));

        if (Separation > ExtentA + ExtentB)
            return false;
    }

    // Now test Cross Products of each basis vector combination ( A[i], B[k] )
    for (i = 0; i < 3; i++)
        for (k = 0; k < 3; k++) {
            int i1 = (i + 1) % 3, i2 = (i + 2) % 3;
            int k1 = (k + 1) % 3, k2 = (k + 2) % 3;
            ExtentA = SizeA[i1] * AR[i2][k] + SizeA[i2] * AR[i1][k];
            ExtentB = SizeB[k1] * AR[i][k2] + SizeB[k2] * AR[i][k1];
            Separation = fabs(vSepA[i2] * R[i1][k] - vSepA[i1] * R[i2][k]);
            if (Separation > ExtentA + ExtentB)
                return false;
        }

    // No separating axis found, the boxes overlap
    return true;
}

glm::vec3 OBox::transformCoords(const glm::vec3 &orig) const
{
    glm::vec4 temp = transform_ * glm::vec4(orig, 1.0f);
    return glm::vec3(temp.x, temp.y, temp.z);
}
