#pragma once

#include <span>
#include <string>

#include <core/numeric-types.h>

namespace Util {

class PhysFSContext {
public:
    explicit PhysFSContext(
        const char *argv0,
        bool mount_base_dir = true,
        bool mount_gtadata = true
    ) noexcept;
    ~PhysFSContext();

    void tryMount(const char *path, bool append_to_path = true) noexcept;
    PhysFSContext &withTryMount(const char *path, bool append_to_path = true) noexcept;

    void mountBaseDir() noexcept;

    bool exists(const char *filename) const noexcept;
};

class PhysFSFile {
public:
    explicit PhysFSFile(const std::string &filename);
    ~PhysFSFile();

    [[nodiscard]] UInt32 length() const noexcept;
    [[nodiscard]] UInt64 tell() const noexcept;
    [[nodiscard]] bool eof() const noexcept;

    void seek(UInt64 pos) noexcept;

    void ensurePosition(UInt64 pos);

    template <BuiltinNumber T>
    void read(T &data) noexcept;

    template <BuiltinNumber T>
    T read() noexcept;

    template <BuiltinNumber T>
    void read(std::span<T> &data) noexcept;

    Int64 read(void *buf, UInt64 len) noexcept;

    std::string readAll() noexcept;

private:
    PHYSFS_File *file;
};

} // namespace Util
