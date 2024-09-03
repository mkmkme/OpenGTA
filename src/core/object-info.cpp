#include <core/object-info.h>

#include <util/file-manager.h>

OpenGTA::ObjectInfo::ObjectInfo(Util::PhysFSFile &file)
{
    file.read(width);
    file.read(height);
    file.read(depth);
    file.read(sprNum);
    file.read(weight);
    file.read(aux);
    file.read(status);
    file.read(numInto);
}
