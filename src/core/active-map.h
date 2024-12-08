#pragma once

#include <memory>
#include <string>

#include <core/map.h>

namespace OpenGTA {
/// The wrapper around the map data interface.
class ActiveMap {
public:
    ActiveMap(const ActiveMap &copy) = delete;
    ActiveMap &operator=(const ActiveMap &copy) = delete;
    ~ActiveMap() = default;

    Map &get();
    void load(const std::string &file);

    static ActiveMap &Instance()
    {
        static ActiveMap instance;
        return instance;
    }

private:
    ActiveMap() = default;
    std::unique_ptr<Map> data_;
};
} // namespace OpenGTA
