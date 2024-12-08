#pragma once

#include <memory>
#include <string>

#include <core/message-db.h>

namespace OpenGTA {
/// The wrapper around the message-string data interface.
class MainMsgLookup {
public:
    MainMsgLookup(const MainMsgLookup &copy) = delete;
    MainMsgLookup &operator=(const MainMsgLookup &copy) = delete;
    ~MainMsgLookup() = default;

    MessageDB &get();
    void load(const std::string &file);

    static MainMsgLookup &Instance()
    {
        static MainMsgLookup instance;
        return instance;
    }

private:
    MainMsgLookup() = default;
    std::unique_ptr<MessageDB> data_;
};
} // namespace OpenGTA
