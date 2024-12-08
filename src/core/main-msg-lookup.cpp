#include <core/main-msg-lookup.h>

#include <util/errors.h>
#include <util/log.h>

namespace OpenGTA {
MessageDB &MainMsgLookup::get()
{
    if (!data_)
        throw Util::NotSupported("Load a resource file first!");
    return *data_;
}

void MainMsgLookup::load(const std::string &file)
{
    INFO("loading message db from {}", file);
    data_ = std::make_unique<MessageDB>(file);
}
} // namespace OpenGTA
