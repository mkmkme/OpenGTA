#include <cassert>
#include <iostream>

#include <core/map.h>
#include <core/navdata.h>

#include <util/file-manager.h>
#include <util/log.h>

int main(int argc, char *argv[])
{
    int x = 0;
    int y = 0;

    if (argc < 2) {
        std::cerr << "USAGE: mapinfo map_filename [x y]" << std::endl;
        return 1;
    }

    Util::PhysFSContext pfs(argv[0]);

    OpenGTA::Map map(argv[1]);

    if (argc == 4) {
        x = strtol(argv[2], nullptr, 10);
        y = strtol(argv[3], nullptr, 10);
    }
    if (x < 0)
        x = 0;
    if (y < 0)
        y = 0;
    INFO("Querying position: {}, {}", x, y);

    OpenGTA::NavData::Sector *sec = map.nav->getSectorAt(x, y);

    INFO("* {} *", sec->name);

    uint16_t num_blocks = map.getNumBlocksAt(x, y);
    INFO("{} empty blocks", num_blocks);
    for (int c = 6 - num_blocks; c >= 1; c--) {
        std::cout << "block " << c << std::endl;
        auto *bi = map.getBlockAt(x, y, c);
        assert(bi);
        std::cout << "moves: " << int(bi->upOk()) << ", " << int(bi->downOk()) << ", " << int(bi->leftOk()) << ", "
                  << int(bi->rightOk()) << std::endl;
        std::cout << "type: " << int(bi->blockType()) << " flat: " << int(bi->isFlat())
                  << " slope-type: " << int(bi->slopeType()) << " rot: " << int(bi->rotation())
                  << " remap idx: " << int(bi->remapIndex()) << std::endl;
        std::cout << "textures: left " << int(bi->left) << " right " << int(bi->right) << " top " << int(bi->top)
                  << " bottom " << int(bi->bottom) << " lid " << int(bi->lid)
                  << " flip t-b: " << int(bi->flipTopBottom()) << " flip l-r: " << int(bi->flipLeftRight())
                  << std::endl;

        int v_count = 0;
        if (bi->lid)
            v_count += 3 * 4;
        if (bi->isFlat()) {
            if (bi->top)
                v_count += 3 * 4 * 2;
            if (bi->left)
                v_count += 3 * 4 * 2;
        } else {
            if (bi->left)
                v_count += 3 * 4;
            if (bi->right)
                v_count += 3 * 4;
            if (bi->top)
                v_count += 3 * 4;
            if (bi->bottom)
                v_count += 3 * 4;
        }
    }

    return 0;
}
