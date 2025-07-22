#include <cassert>
#include <string>

#include <core/car-info.h>
#include <core/graphics-8bit.h>
#include <core/map.h>
#include <core/sprite-info.h>
#include <fmt/base.h>

#include <util/file-manager.h>

void dump_in_map(const std::string &style_file, const std::string &map_file)
{
    OpenGTA::Map map(map_file);
    OpenGTA::Graphics8Bit style(style_file);

    for (int i = 0; i < map.numObjects; i++) {
        OpenGTA::Map::ObjectPosition &o = map.objects[i];
        fmt::println("object: {}", i);
        fmt::println("{}, {}, {}, rot: {}, pitch: {}, roll: {}", o.x, o.y, o.z, o.rotation, o.pitch, o.roll);
        fmt::print("type: {}", o.type);
        if (o.remap >= 128) {
            fmt::println(" (a car) remap: {}", o.remap - 128);
            auto &info = style.findCarByModel(o.type);
            fmt::println(
                "width: {}, height: {}, depth: {}, sprnum: {}, weight: {}",
                info.width,
                info.height,
                info.depth,
                info.sprNum,
                info.weightDescriptor
            );
        } else {
            fmt::println(" (an obj) remap: {}", o.remap);
            const auto &info = style.objectInfos[o.type];
            fmt::println(
                "width: {}, height: {}, depth: {}, sprnum: {}, weight: {}, aux: {}, status: {}, n-into: {}",
                info.width,
                info.height,
                info.depth,
                info.sprNum,
                info.weight,
                info.aux,
                int(info.status),
                int(info.numInto)
            );
        }
        fmt::print("\n");
    }
}

void dump(const std::string &style_file)
{
    OpenGTA::Graphics8Bit style(style_file);
    fmt::println("DUMP_OBJ_INFO BEGIN");
    for (size_t i = 0; i < style.objectInfos.size(); ++i) {
        fmt::println(
            "obj-type: {}, width: {}, height: {}, depth: {}, spr-num: {}, reindex: {}, weight: {}, aux: {}, status: "
            "{}, num-into: {}",
            i,
            style.objectInfos[i].width,
            style.objectInfos[i].height,
            style.objectInfos[i].depth,
            style.objectInfos[i].sprNum,
            style.spriteNumbers
                .reIndex(style.objectInfos[i].sprNum, OpenGTA::GraphicsBase::SpriteNumbers::SpriteTypes::object),
            style.objectInfos[i].weight,
            style.objectInfos[i].aux,
            int(style.objectInfos[i].status),
            int(style.objectInfos[i].numInto)
        );
    }
    fmt::println("DUMP_SPRITE_INFO END");
    for (size_t i = 0; i < style.spriteInfos.size(); ++i) {
        fmt::println(
            "sprite: {}, width: {}, height: {}, delta-count: {}",
            i,
            int(style.spriteInfos[i].w),
            int(style.spriteInfos[i].h),
            int(style.spriteInfos[i].deltaCount)
        );
    }
}

int main(int argc, char *argv[])
{
    if (argc < 2 || argc > 3) {
        fmt::println(stderr, "USAGE: {} GRY_FILE [MAP_FILE]", argv[0]);
        return 1;
    }

    const std::string style_file = argv[1];
    std::string map_file {};
    if (argc == 3) {
        map_file = argv[2];
    }

    Util::PhysFSContext pfs { argv[0] };

    if (!map_file.empty()) {
        dump_in_map(style_file, map_file);
    } else {
        dump(style_file);
    }

    return 0;
}
