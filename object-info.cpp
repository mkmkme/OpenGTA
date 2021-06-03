#include "object-info.h"

OpenGTA::ObjectInfo::ObjectInfo(PHYSFS_file *fd)
{
    PHYSFS_readULE32(fd, &width);
    PHYSFS_readULE32(fd, &height);
    PHYSFS_readULE32(fd, &depth);
    PHYSFS_readULE16(fd, &sprNum);
    PHYSFS_readULE16(fd, &weight);
    PHYSFS_readULE16(fd, &aux);
    PHYSFS_readBytes(fd, static_cast<void*>(&status), 1);
    PHYSFS_readBytes(fd, static_cast<void*>(&numInto), 1);
}
