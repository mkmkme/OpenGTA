#include "car-info.h"
#include "dataholder.h"

#include <iostream>
#include <string>

void print_car(OpenGTA::CarInfo &ci)
{
    std::string model = "car" + std::to_string(int(ci.model));
#define PRINT(c)  #c << ":" << ci.c << "|"
#define PRINTC(c) #c << ":" << int(ci.c) << "|"
    std::cout << PRINT(width) << PRINT(height) << PRINT(depth) << PRINT(sprNum) << PRINT(weightDescriptor)
              << PRINT(maxSpeed) << PRINT(minSpeed) << PRINT(acceleration) << PRINT(braking) << PRINT(grip)
              << PRINT(handling)
              // remaps
              << PRINTC(vtype) << PRINTC(model) << PRINTC(turning) << PRINTC(damagable)
              << "model-name:" << OpenGTA::MainMsgLookup::Instance().get().getText(model) << "|" << PRINTC(cx)
              << PRINTC(cy) << PRINT(moment) << PRINT(rbpMass) << PRINT(g1_Thrust) << PRINT(tyreAdhesionX)
              << PRINT(tyreAdhesionY) << PRINT(handBrakeFriction) << PRINT(footBrakeFriction) << PRINT(frontBrakeBias)
              << PRINT(turnRatio) << PRINT(driveWheelOffset) << PRINT(steeringWheelOffset) << PRINT(backEndSlideValue)
              << PRINT(handBrakeSlideValue) << PRINTC(convertible) << PRINTC(engine) << PRINTC(radio) << PRINTC(horn)
              << PRINTC(soundFunction) << PRINTC(fastChangeFlag) << PRINT(numDoors);
#undef PRINT
#define PRINT(c) #c << ":" << ci.door[i].c << "|"
    for (int i = 0; i < ci.numDoors; i++) {
        std::cout << PRINT(rpx) << PRINT(rpy) << PRINT(object) << PRINT(delta);
    }
    std::cout << "remap8:";
    for (int i = 0; i < 12; i++) {
        std::cout << int(ci.remap8[i]) << (i < 11 ? "," : "");
    }
    std::cout << std::endl;
#undef PRINT
}

int main(int argc, char *argv[])
{
    if (argc != 2) {
        std::cerr << "USAGE: " << argv[0] << " STYLE_FILE" << std::endl;
        return 1;
    }

    const std::string style_file = argv[1];

    PHYSFS_init("mapview");
    PHYSFS_mount(PHYSFS_getBaseDir(), nullptr, 1);
    PHYSFS_mount("gtadata.zip", nullptr, 1);

    OpenGTA::MainMsgLookup::Instance().load("ENGLISH.FXT");
    OpenGTA::ActiveStyle::Instance().load(style_file);
    OpenGTA::GraphicsBase &style = OpenGTA::ActiveStyle::Instance().get();
    std::cout << "DUMP_OBJ_INFO BEGIN" << std::endl;
    for (auto &cinfo : style.carInfos) {
        print_car(cinfo);
    }

    PHYSFS_deinit();
    return 0;
}