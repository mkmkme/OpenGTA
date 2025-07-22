#ifndef MAP_HELPER_H
#define MAP_HELPER_H

#include <list>
#include <map>
#include <random>

#include <SDL2/SDL_video.h>
#include <glm/ext/vector_float3.hpp>

namespace Util {
using TupleOfRects = std::pair<SDL_Rect, SDL_Rect>;
using TupleOfUint8 = std::pair<uint8_t, uint8_t>;
using ListOfTupleOfRects = std::list<TupleOfRects>;

class SpriteCreationArea {
public:
    SpriteCreationArea();
    // ListOfTupleOfRects validTuple;
    TupleOfRects validRects;
    void setRects(const SDL_Rect &allowed, const SDL_Rect &denied);
    TupleOfUint8 getValidCoord();
    [[nodiscard]] bool isOnScreen(const glm::vec3 &p) const;

private:
    SDL_Rect onScreen;
    std::mt19937 rng_;
};

using Pair2Int = std::pair<int, int>;
using MapOfPair2Int = std::map<Pair2Int, int>;

/** count of Pair2Int(a,b) in the map.
 * @return 0 if no entry at that \e position
 * @return count otherwise
 */
int item_count(MapOfPair2Int &m, int a, int b);
/** insert (may overwrite) count 1 at Pair2Int(a, b).
 */
void register_item1(MapOfPair2Int &m, int a, int b);
/** register and count Pair2Int(a, b).
 */
void register_item(MapOfPair2Int &m, int a, int b);

} // namespace Util
#endif
