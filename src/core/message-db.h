#pragma once

#include <cstdint>
#include <map>
#include <string>

namespace OpenGTA {

class MessageDB {
public:
    explicit MessageDB(const std::string &file);
    void load(const std::string &file);
    const std::string &getText(const std::string &id);
    const std::string &getText(uint32_t id);

private:
    MessageDB();
    std::map<std::string, std::string> messages;
    std::string _error;
};

} // namespace OpenGTA
