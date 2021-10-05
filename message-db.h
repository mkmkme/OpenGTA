#pragma once

#include <map>
#include <string>

namespace OpenGTA {

class MessageDB {
public:
    MessageDB();
    MessageDB(const std::string &file);
    ~MessageDB();
    void load(const std::string &file);
    const std::string &getText(const std::string &id);
    const std::string &getText(const uint32_t id);

private:
    std::map<std::string, std::string> messages;
    std::string _error;
};

} // namespace OpenGTA
