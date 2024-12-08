#pragma once

#include <memory>
#include <string>

#include <core/graphics-base.h>

namespace OpenGTA {
/// The wrapper around the GRY/G24 data interface.
class ActiveStyle {
public:
    ActiveStyle(const ActiveStyle &copy) = delete;
    ActiveStyle &operator=(const ActiveStyle &copy) = delete;
    ~ActiveStyle() = default;

    GraphicsBase &get();
    void load(const std::string &file);

    static ActiveStyle &Instance()
    {
        static ActiveStyle instance;
        return instance;
    }

private:
    ActiveStyle() = default;
    std::unique_ptr<GraphicsBase> data_;
};
} // namespace OpenGTA
