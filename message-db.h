#pragma once

#include <cstdint>
#include <map>
#include <string>

namespace OpenGTA {

class MessageDB final {
private:
    explicit MessageDB(const std::string &file);
public:
    static std::unique_ptr<MessageDB> create(const std::string &file);
    void load(const std::string &file);
    [[nodiscard]] const std::string &getText(const std::string &id) const noexcept;
    [[nodiscard]] const std::string &getText(uint32_t id) const noexcept;

private:
    std::map<std::string, std::string> messages;
    std::string _error = "ERROR";
};

} // namespace OpenGTA
