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

#include <array>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/mat3x3.hpp>
#include <glm/vec3.hpp>

class OBox {
public:
    OBox() = default;
    OBox(glm::mat4 m, glm::vec3 extent)
        : transform_(m)
        , extent_(extent)
    {
    }
    OBox(glm::mat4 m, glm::vec3 low, glm::vec3 high)
        : transform_(glm::translate(m, 0.5f * (low + high)))
        , extent_(0.5f * (high - low))
    {
    }
    OBox(const OBox &other) = default;
    OBox(OBox &&other) noexcept = default;

    [[nodiscard]] glm::vec3 getSize() const noexcept { return 2.0f * extent_; }
    [[nodiscard]] glm::vec3 getCenterPoint() const noexcept
    {
        return { transform_[3][0], transform_[3][1], transform_[3][2] };
    }

    [[nodiscard]] bool isPointInBox(const glm::vec3 &p) const;
    bool isBoxInBox(OBox &box) const;
    [[nodiscard]] bool isSphereInBox(const glm::vec3 &p, float fRadius) const;
    [[nodiscard]] bool isLineInBox(const glm::vec3 &l1, const glm::vec3 &l2) const;
    [[nodiscard]] bool boxOutsidePlane(const glm::vec3 &normal, const glm::vec3 &p) const;

    void lineCrossBox(const glm::vec3 &l1, const glm::vec3 &l2, glm::vec3 &isecLocalSpace) const;

    [[nodiscard]] const glm::mat4 &transform() const noexcept { return transform_; }

    [[nodiscard]] const glm::vec3 &extent() const noexcept { return extent_; }

    [[nodiscard]] glm::vec3 transformCoords(const glm::vec3 &orig) const;

private:
    [[nodiscard]] std::array<glm::vec3, 3> getInvRot() const;
    // Data
    // Matrix3D m_M;
    // Vector3D m_Extent;
protected:
    glm::mat4 transform_;
    glm::vec3 extent_;
};
