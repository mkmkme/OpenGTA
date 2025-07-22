#pragma once

#include <concepts>
#include <cstdint>
#include <span>
#include <string>

#include <physfs.h>

namespace Util {

class PhysFSContext {
public:
    explicit PhysFSContext(const char *argv0, bool mount_base_dir = true, bool mount_gtadata = true) noexcept;
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

    [[nodiscard]] uint32_t length() const noexcept;
    [[nodiscard]] uint64_t tell() const noexcept;
    [[nodiscard]] bool eof() const noexcept;

    void seek(uint64_t pos) noexcept;

    void ensurePosition(uint64_t pos);

    template <std::integral T>
    void read(T &data) noexcept;

    template <std::integral T>
    T read() noexcept;

    template <std::integral T>
    void read(std::span<T> &data) noexcept;

    int64_t read(void *buf, uint64_t len) noexcept;

    std::string readAll() noexcept;

private:
    PHYSFS_File *file;
};

} // namespace Util
