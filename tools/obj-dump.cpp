#include "car-info.h"
#include "graphics-8bit.h"
#include "map.h"
#include "sprite-info.h"

#include <cassert>
#include <iostream>
#include <string>

void dump_in_map(const std::string &style_file, const std::string &map_file)
{
    OpenGTA::Map map(map_file);
    OpenGTA::Graphics8Bit style(style_file);

    for (int i = 0; i < map.numObjects; i++) {
        OpenGTA::Map::ObjectPosition &o = map.objects[i];
        std::cout << "object: " << i << std::endl;
        std::cout << o.x << ", " << o.y << ", " << o.z << " rot: " << o.rotation << " pitch: " << o.pitch
                  << " roll: " << o.roll << std::endl;
        std::cout << "type: " << int(o.type);
        if (o.remap >= 128) {
            std::cout << " (a car) remap: " << int(o.remap - 128) << std::endl;
            auto &info = style.findCarByModel(o.type);
            std::cout << "width: " << info.width << " height: " << info.height << " depth " << info.depth
                      << " sprnum: " << info.sprNum << " weight: " << info.weightDescriptor << std::endl;
        } else {
            std::cout << " (an obj) remap: " << int(o.remap) << std::endl;
            const auto &info = style.objectInfos[o.type];
            std::cout << "width: " << info.width << " height: " << info.height << " depth " << info.depth
                      << " sprnum: " << info.sprNum << " weight: " << info.weight << " aux: " << info.aux
                      << " status: " << int(info.status) << " n-into: " << int(info.numInto) << std::endl;
        }
        std::cout << std::endl;
    }
}

void dump(const std::string &style_file)
{
    OpenGTA::Graphics8Bit style(style_file);
    std::cout << "DUMP_OBJ_INFO BEGIN" << std::endl;
    for (size_t i = 0; i < style.objectInfos.size(); ++i) {
        std::cout
            << "obj-type: " << i << "  width: " << style.objectInfos[i].width
            << " height: " << style.objectInfos[i].height << " depth: " << style.objectInfos[i].depth
            << " spr-num: " << style.objectInfos[i].sprNum << " reindex: "
            << style.spriteNumbers.reIndex(style.objectInfos[i].sprNum, OpenGTA::GraphicsBase::SpriteNumbers::OBJECT)
            << " weight: " << style.objectInfos[i].weight << " aux: " << style.objectInfos[i].aux
            << " status: " << int(style.objectInfos[i].status) << " num-into: " << int(style.objectInfos[i].numInto)
            << std::endl;
    }
    std::cout << "DUMP_OBJ_INFO END" << std::endl;
    for (size_t i = 0; i < style.spriteInfos.size(); ++i) {
        std::cout << "sprite: " << i << "  width: " << int(style.spriteInfos[i]->w)
                  << " height: " << int(style.spriteInfos[i]->h)
                  << " delta-count: " << int(style.spriteInfos[i]->deltaCount) << std::endl;
    }
}

int main(int argc, char *argv[])
{
    if (argc < 2 || argc > 3) {
        std::cerr << "USAGE: " << argv[0] << " GRY_FILE [MAP_FILE]" << std::endl;
        return 1;
    }

    const std::string style_file = argv[1];
    std::string map_file {};
    if (argc == 3) {
        map_file = argv[2];
    }

    PHYSFS_init("mapview");
    PHYSFS_mount(PHYSFS_getBaseDir(), nullptr, 1);
    PHYSFS_mount("gtadata.zip", nullptr, 1);

    if (!map_file.empty()) {
        dump_in_map(style_file, map_file);
    } else {
        dump(style_file);
    }

    PHYSFS_deinit();
    return 0;
}