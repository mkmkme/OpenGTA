#include "file-manager.h"

#include <filesystem>

#include <physfs.h>

#include <core/numeric-types.h>
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
            throw FileNotFound(
                filename + " with error: " + PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode())
            );
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

template <BuiltinNumber T>
void PhysFSFile::read(T &data) noexcept
{
    if constexpr (std::is_same_v<T, UInt8> || std::is_same_v<T, Int8>) {
        PHYSFS_readBytes(file, static_cast<void *>(&data), 1);
    } else if constexpr (std::is_same_v<T, Int16>) {
        PHYSFS_readSLE16(file, &data);
    } else if constexpr (std::is_same_v<T, UInt16>) {
        PHYSFS_readULE16(file, &data);
    } else if constexpr (std::is_same_v<T, UInt32>) {
        PHYSFS_readULE32(file, &data);
    } else {
        static_assert(false, "Unsupported type");
    }
}

template <BuiltinNumber T>
T PhysFSFile::read() noexcept
{
    T data;
    read(data);
    return data;
}

template <typename T, size_t N>
void PhysFSFile::read(std::span<T, N> &data) noexcept
{
    PHYSFS_readBytes(file, data.data(), N * sizeof(T));
}

Int64 PhysFSFile::read(void *buf, UInt64 len) noexcept
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

UInt32 PhysFSFile::length() const noexcept
{
    return PHYSFS_fileLength(file);
}

bool PhysFSFile::eof() const noexcept
{
    return PHYSFS_eof(file);
}

UInt64 PhysFSFile::tell() const noexcept
{
    return PHYSFS_tell(file);
}

void PhysFSFile::seek(UInt64 pos) noexcept
{
    PHYSFS_seek(file, pos);
}

void PhysFSFile::ensurePosition(UInt64 pos)
{
    const auto curPos = PHYSFS_tell(file);
    if (curPos != pos)
        throw std::runtime_error(fmt::format("File position mismatch: {} != {} (expected)", curPos, pos));
}

template void PhysFSFile::read<Int8>(Int8 &);
template void PhysFSFile::read<UInt8>(UInt8 &);
template void PhysFSFile::read<Int16>(Int16 &);
template void PhysFSFile::read<UInt16>(UInt16 &);
template void PhysFSFile::read<UInt32>(UInt32 &);

template Int8 PhysFSFile::read<Int8>();
template UInt8 PhysFSFile::read<UInt8>();
template Int16 PhysFSFile::read<Int16>();
template UInt16 PhysFSFile::read<UInt16>();
template UInt32 PhysFSFile::read<UInt32>();

} // namespace Util
