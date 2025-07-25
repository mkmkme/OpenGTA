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
#ifndef TEXTURECACHE_H
#define TEXTURECACHE_H
#include <map>
#include <string>

#include <SDL2/SDL_opengl.h>

namespace OpenGL {

/** Helper for the render code.
 *
 * This class handles the storage of GLuint texture ids and
 * provides a mapping from the internal ids used by the map files.
 */
template <typename key_type>
class TextureCache {
public:
    /** Simple constructor.
     *
     * Name will be set to TextureCache_N, where N is the current instance count.
     */
    TextureCache() noexcept;

    /** Constructor with a name.
     *
     * Set a name for this instance; to clarify status output with multiple caches.
     */
    explicit TextureCache(const char *with_name) noexcept;

    /** Just a simple destructor, nothing see here... move along.
     *
     * As a sidenote: this calls 'glDeleteTextures' on the images
     * stored inside.
     */
    ~TextureCache();

    /** Check if texture is already cached.
     *
     * @param id key from map/block info
     * @return true if texture is found
     * @return false otherwise
     */
    bool hasTexture(key_type id) noexcept;

    /** Maps internal id to GLuint texture id.
     *
     * @param id from map/block info
     * @return texture id
     */
    GLuint getTextureWithId(key_type id) noexcept;

    /** Adds a texture to the cache and maps to internal id.
     *
     * @param id requested internal id from map/block info
     * @param texId texture id that contains the image
     */
    void addTexture(key_type id, GLuint texId);

    /** Set specified texture to hasAlpha.
     *
     * This doesn't do anything; you can just check for hasAlpha later on.
     */
    void setToAlpha(key_type id) noexcept;

    /** probably stupid idea/going to go away
     */
    void setToAnimated(key_type id) noexcept;

    /** Dumps some status info to stdout.
     */
    void status() noexcept;

    /** Iterate over stored textures and modify refCount.
     *
     * This is optional functionality; you may skip this. If you don't,
     * call this *before* each rendering pass.
     */
    void sink() noexcept;

    /** Remove unused textures from cache and video memory.
     *
     * Call this *after* a rendering pass, but not every frame!
     *
     * Handle with care, this code is experimental.
     */
    void clear() noexcept;

    void clearAll() noexcept;

    void clearStats() noexcept;
    void printStats() noexcept;

    void setClearMagic(uint32_t removeLesser) noexcept;
    void setMinClearElements(uint32_t minElements) noexcept;

protected:
    struct texTuple;
    using CacheMapType = std::map<key_type, texTuple>;

    unsigned int clearMagic;
    unsigned int minClearElements;

    struct texTuple {
        GLuint texId;
        uint32_t refCount;
        bool hasAlpha;
        bool isAnimated;
    };
    CacheMapType cached;
    std::string m_name;
    static unsigned int instance_count;
    unsigned int instance_id;
    bool has_cached_query;
    key_type last_query_id;
    texTuple *last_query_result;
    [[nodiscard]] bool matchingCachedQuery(key_type id) const noexcept;
    void cacheQuery(key_type id, texTuple *pos) noexcept;
};

} // namespace OpenGL

#endif
