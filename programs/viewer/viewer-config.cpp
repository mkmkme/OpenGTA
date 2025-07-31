#include "viewer-config.h"

#include <algorithm>
#include <array>

#include <lua.h>

#include <cxxopts.hpp>

#include "lua-addon/stackguard.h"
#include "util/log.h"

#include "version-info.h"

namespace {
struct CityInfo {
    std::string name;
    std::string style;

    [[nodiscard]] std::string getMapFile() const noexcept { return fmt::format("{}.CMP", name); }
    [[nodiscard]] std::string getStyleFile(bool highcolor) const noexcept
    {
        return fmt::format("{}.{}", style, highcolor ? "G24" : "GRY");
    }
};

constexpr std::array cities = {
    CityInfo { .name = "NYC", .style = "STYLE001" },
    CityInfo { .name = "SANB", .style = "STYLE002" },
    CityInfo { .name = "MIAMI", .style = "STYLE003" },
};

} // namespace

namespace OpenGTA {

ViewerConfig::ViewerConfig() = default;

ViewerConfig::ParseArgsResult ViewerConfig::parseArgs(int argc, char **argv)
{
    progname_ = argv[0];

    cxxopts::Options options { "viewer", "Demo program for OpenGTA" };
    // clang-format off
    options.add_options()
        ("s,script", "Path to Lua script to execute", cxxopts::value<std::string>(script_file_))
        ("a", "Anisotropic filter degree: 1.0 = disabled", cxxopts::value<float>(anisotropic_filter_degree_))
        ("c,flip-colors", "Flip color mode: 8bit GRY if 24bit G24 is default, G24 if GRY is default", cxxopts::value<bool>())
        ("m,map", "Load specified map", cxxopts::value<std::string>(map_name_))
        ("M,mipmaps", "Enable texture mipmaps", cxxopts::value<bool>(mipmap_textures_))
        ("g", "Load specified style", cxxopts::value<std::string>(style_name_))
        ("w,width", "Set screen width", cxxopts::value<uint32_t>(window_width_))
        ("h,height", "Set screen height", cxxopts::value<uint32_t>(window_height_))
        ("l,log-level", "Log level (error, warn, info, debug) (default: info)", cxxopts::value<std::string>()->default_value("info"))
        ("f,fullscreen", "Fullscreen mode on start (broken)", cxxopts::value<bool>(fullscreen_))
        ("vsync", "Enable vertical sync", cxxopts::value<bool>(vsync_))
        ("V,version", "Print version and exit")
        ("x,scale2x", "Enable scale2x sprites", cxxopts::value<bool>(scale2x_))
        ("city", "City to start with (NYC, SANB, MIAMI)", cxxopts::value<std::string>())
        ("help", "Print help and exit")
        ;
    // clang-format on

    try {
        auto result = options.parse(argc, argv);
        if (result.count("help")) {
            fmt::print("{}", options.help());
            return ParseArgsResult::ExitSuccess;
        }
        if (result.count("version")) {
            Util::printVersionInfo();
            return ParseArgsResult::ExitSuccess;
        }
        if (result.count("l")) {
            auto log_level = result["log-level"].as<std::string>();
            const std::map<std::string, OpenGTA::log::Level> log_levels = {
                { "error", OpenGTA::log::Level::error },
                { "warn", OpenGTA::log::Level::warn },
                { "info", OpenGTA::log::Level::info },
                { "debug", OpenGTA::log::Level::debug },
            };
            const auto it = log_levels.find(log_level);
            if (it == log_levels.end()) {
                fmt::print(stderr, "Invalid log level, falling back to info");
                OpenGTA::log::level = OpenGTA::log::Level::info;
            } else {
                OpenGTA::log::level = it->second;
            }
        }
        if (result.count("flip-colors")) {
            highcolor_data_ = !highcolor_data_;
        }

        if (result.count("city")) {
            auto city_name = result["city"].as<std::string>();
            if (const auto it = std::ranges::find(cities, city_name, &CityInfo::name); it == cities.end()) {
                ERROR("City {} not found, falling back to NYC", city_name);
                city_id_ = 0;
            } else {
                city_id_ = std::ranges::distance(cities.begin(), it);
            }
        }

    } catch (const cxxopts::exceptions::exception &e) {
        fmt::println(stderr, "Error parsing options: {}", e.what());
        return ParseArgsResult::ExitError;
    }
    return ParseArgsResult::Success;
}

void ViewerConfig::tryParseLuaConfig(Script::LuaVM &luaVM) noexcept
{
    auto *L = luaVM.getInternalState();

    lua_getglobal(L, "config");
    if (lua_type(L, 1) == LUA_TTABLE) {
        luaVM.tryGetBool("use_g24_graphics", highcolor_data_);
        luaVM.tryGetUInt("screen_width", window_width_);
        luaVM.tryGetUInt("screen_height", window_height_);
        luaVM.tryGetBool("screen_vsync", vsync_);
        luaVM.tryGetBool("full_screen", fullscreen_);

        luaVM.tryGetFloat("gl_field_of_view", fov_);
        luaVM.tryGetFloat("gl_near_plane", near_plane_);
        luaVM.tryGetFloat("gl_far_plane", far_plane_);
        luaVM.tryGetBool("gl_mipmap_textures", mipmap_textures_);

        luaVM.tryGetBool("scale2x_sprites", scale2x_);

        luaVM.tryGetFloat("gl_anisotropic_textures", anisotropic_filter_degree_);

        if (highcolor_data_) {
            luaVM.tryGetFloat("screen_gamma_g24", screen_gamma_);
        } else {
            luaVM.tryGetFloat("screen_gamma_gry", screen_gamma_);
        }
    }
    lua_settop(L, 0);
    // can't check for gl-extensions now
}

void ViewerConfig::saveLuaConfig(Script::LuaVM &luaVM) const noexcept
{
    auto *L = luaVM.getInternalState();
    ::Util::LuaStackguard guard(L);

    lua_createtable(L, 0, 4);

    lua_pushboolean(L, highcolor_data_);
    lua_setfield(L, -2, "use_g24_graphics");

    lua_pushinteger(L, window_width_);
    lua_setfield(L, -2, "screen_width");

    lua_pushinteger(L, window_height_);
    lua_setfield(L, -2, "screen_height");

    lua_pushboolean(L, vsync_);
    lua_setfield(L, -2, "screen_vsync");

    lua_pushboolean(L, fullscreen_);
    lua_setfield(L, -2, "full_screen");

    lua_pushnumber(L, fov_);
    lua_setfield(L, -2, "gl_field_of_view");

    lua_pushnumber(L, near_plane_);
    lua_setfield(L, -2, "gl_near_plane");

    lua_pushnumber(L, far_plane_);
    lua_setfield(L, -2, "gl_far_plane");

    lua_pushboolean(L, mipmap_textures_);
    lua_setfield(L, -2, "gl_mipmap_textures");

    lua_pushboolean(L, scale2x_);
    lua_setfield(L, -2, "scale2x_sprites");

    lua_pushnumber(L, anisotropic_filter_degree_);
    lua_setfield(L, -2, "gl_anisotropic_textures");

    if (highcolor_data_) {
        lua_pushnumber(L, screen_gamma_);
        lua_setfield(L, -2, "screen_gamma_g24");
    } else {
        lua_pushnumber(L, screen_gamma_);
        lua_setfield(L, -2, "screen_gamma_gry");
    }

    lua_pushvalue(L, -1);
    lua_setglobal(L, "config");

    lua_settop(L, 0);
}

std::string ViewerConfig::getMapFile() const noexcept
{
    return cities[city_id_].getMapFile();
}

std::string ViewerConfig::getStyleFile() const noexcept
{
    return cities[city_id_].getStyleFile(highcolor_data_);
}

} // namespace OpenGTA
