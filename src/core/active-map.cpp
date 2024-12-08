#include <core/active-map.h>

#include <util/errors.h>
#include <util/log.h>

namespace OpenGTA {
Map &ActiveMap::get()
{
    if (!data_)
        throw Util::NotSupported("Load a resource file first!");
    return *data_;
}

void ActiveMap::load(const std::string &file)
{
    INFO("loading map from {}", file);
    data_ = std::make_unique<Map>(file);
}

} // namespace OpenGTA
