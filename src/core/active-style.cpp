#include <core/active-style.h>
#include <core/graphics-24bit.h>
#include <core/graphics-8bit.h>

#include <util/errors.h>
#include <util/log.h>
#include <util/string_helpers.h>


namespace OpenGTA {
GraphicsBase &ActiveStyle::get()
{
    if (!data_)
        throw Util::NotSupported("Load a resource file first!");
    return *data_;
}

void ActiveStyle::load(const std::string &file)
{
    std::string tempName { Util::string_lower(file) };
    INFO("loading style from {}", file);
    if (tempName.find(".g24") != std::string::npos) {
        data_ = std::make_unique<Graphics24Bit>(file);
    } else if (tempName.find(".gry") != std::string::npos) {
        data_ = std::make_unique<Graphics8Bit>(file);
    } else {
        try {
            data_ = std::make_unique<Graphics8Bit>(file);
        } catch (const Util::Exception &e) {
            WARN("loading 8 bit failed: {}", e.what());
            data_ = std::make_unique<Graphics24Bit>(file);
        }
    }
}

} // namespace OpenGTA
