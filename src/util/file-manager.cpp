#include "file-manager.h"

#include <filesystem>

#include <physfs.h>

#include <fmt/core.h>
#include <fmt/format.h>

#include <util/errors.h>
#include <util/file_helper.h>
#include <util/log.h>
#include <util/string_helpers.h>

namespace {
struct {
    size_t fileCount = 0;
    bool isInitialized = false;
    bool lateDestroy = false;
} global_context;
} // namespace

namespace Util {

PhysFSContext::PhysFSContext(const char *argv0, bool mount_base_dir, bool mount_gtadata) noexcept
{
    PHYSFS_init(argv0);
    if (mount_base_dir)
        mountBaseDir();
    if (mount_gtadata)
        tryMount(FileHelper::BaseDataPath().c_str());
    global_context.isInitialized = true;
}

PhysFSContext::~PhysFSContext()
{
    if (global_context.fileCount > 0) {
        WARN("There are still {} files open", global_context.fileCount);
        global_context.lateDestroy = true;
    } else {
        INFO("All files closed, deinitializing PhysFS");
        PHYSFS_deinit();
        global_context.isInitialized = false;
    }
}

void PhysFSContext::mountBaseDir() noexcept
{
    PHYSFS_mount(PHYSFS_getBaseDir(), nullptr, 1);
}

void PhysFSContext::tryMount(const char *path, bool append_to_path) noexcept
{
    if (std::filesystem::exists(path)) {
        PHYSFS_mount(path, nullptr, append_to_path);
    }
}

PhysFSContext &PhysFSContext::withTryMount(const char *path, bool append_to_path) noexcept
{
    tryMount(path, append_to_path);
    return *this;
}

bool PhysFSContext::exists(const char *filename) const noexcept
{
    return PHYSFS_exists(filename);
}

PhysFSFile::PhysFSFile(const std::string &filename)
    : file { PHYSFS_openRead(filename.c_str()) }
{
    if (!global_context.isInitialized)
        throw std::runtime_error("PhysFS not initialized");
    if (file == nullptr) {
        std::string filename_lower { string_lower(filename) };
        file = PHYSFS_openRead(filename_lower.c_str());
        if (file == nullptr)
            throw FileNotFound(filename + " with error: " + PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()));
    }
    ++global_context.fileCount;
}

PhysFSFile::~PhysFSFile()
{
    PHYSFS_close(file);
    --global_context.fileCount;
    if (global_context.lateDestroy && global_context.fileCount == 0) {
        INFO("All files closed, late-deinitializing PhysFS");
        PHYSFS_deinit();
    }
}

template <std::integral T>
void PhysFSFile::read(T &data) noexcept
{
    if constexpr (std::is_same_v<T, uint8_t> || std::is_same_v<T, int8_t>) {
        PHYSFS_readBytes(file, static_cast<void *>(&data), 1);
    } else if constexpr (std::is_same_v<T, int16_t>) {
        PHYSFS_readSLE16(file, &data);
    } else if constexpr (std::is_same_v<T, uint16_t>) {
        PHYSFS_readULE16(file, &data);
    } else if constexpr (std::is_same_v<T, uint32_t>) {
        PHYSFS_readULE32(file, &data);
    } else {
        static_assert(false, "Unsupported type");
    }
}

template <std::integral T>
T PhysFSFile::read() noexcept
{
    T data;
    read(data);
    return data;
}

template <std::integral T>
void PhysFSFile::read(std::span<T> &data) noexcept
{
    PHYSFS_readBytes(file, data.data(), data.size_bytes());
}

int64_t PhysFSFile::read(void *buf, uint64_t len) noexcept
{
    return PHYSFS_readBytes(file, buf, len);
}

std::string PhysFSFile::readAll() noexcept
{
    const auto len = length();
    std::string buf(len, '\0');
    read(buf.data(), len);
    return buf;
}

uint32_t PhysFSFile::length() const noexcept
{
    return PHYSFS_fileLength(file);
}

bool PhysFSFile::eof() const noexcept
{
    return PHYSFS_eof(file);
}

uint64_t PhysFSFile::tell() const noexcept
{
    return PHYSFS_tell(file);
}

void PhysFSFile::seek(uint64_t pos) noexcept
{
    PHYSFS_seek(file, pos);
}

void PhysFSFile::ensurePosition(uint64_t pos)
{
    const auto curPos = PHYSFS_tell(file);
    if (curPos != pos)
        throw std::runtime_error(fmt::format("File position mismatch: {} != {} (expected)", curPos, pos));
}

template void PhysFSFile::read<int8_t>(int8_t &);
template void PhysFSFile::read<uint8_t>(uint8_t &);
template void PhysFSFile::read<int16_t>(int16_t &);
template void PhysFSFile::read<uint16_t>(uint16_t &);
template void PhysFSFile::read<uint32_t>(uint32_t &);

template int8_t PhysFSFile::read<int8_t>();
template uint8_t PhysFSFile::read<uint8_t>();
template int16_t PhysFSFile::read<int16_t>();
template uint16_t PhysFSFile::read<uint16_t>();
template uint32_t PhysFSFile::read<uint32_t>();

template void PhysFSFile::read<uint8_t>(std::span<uint8_t> &);
template void PhysFSFile::read<uint16_t>(std::span<uint16_t> &);

} // namespace Util
