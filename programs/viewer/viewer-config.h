#pragma once

#include <cstdint>
#include <string>

#include "lua-addon/vm.h"

#include "base/config.h"

namespace OpenGTA {

class ViewerConfig {
public:
    enum class ParseArgsResult : uint8_t { Success, ExitSuccess, ExitError };

    ViewerConfig();

    ParseArgsResult parseArgs(int argc, char **argv);
    void tryParseLuaConfig(Script::LuaVM &luaVM) noexcept;

    void saveLuaConfig(Script::LuaVM &luaVM) const noexcept;

    [[nodiscard]] std::string_view getProgName() const noexcept { return progname_; }

    [[nodiscard]] std::string getMapFile() const noexcept;
    [[nodiscard]] std::string getStyleFile() const noexcept;

    [[nodiscard]] uint32_t getWindowWidth() const noexcept { return window_width_; }
    [[nodiscard]] uint32_t getWindowHeight() const noexcept { return window_height_; }
    [[nodiscard]] bool isHighcolorData() const noexcept { return highcolor_data_; }
    [[nodiscard]] bool isFullscreen() const noexcept { return fullscreen_; }
    [[nodiscard]] bool isVsync() const noexcept { return vsync_; }
    [[nodiscard]] bool mipmapTextures() const noexcept { return mipmap_textures_; }
    [[nodiscard]] bool isScale2x() const noexcept { return scale2x_; }
    [[nodiscard]] std::string getScriptFile() const noexcept { return script_file_; }

    [[nodiscard]] float getAnisotropicFilterDegree() const noexcept { return anisotropic_filter_degree_; }
    [[nodiscard]] float getScreenGamma() const noexcept { return screen_gamma_; }

private:
    std::string_view progname_;

    // args start
    std::string script_file_;
    float anisotropic_filter_degree_ = 2.0f;
    std::string map_name_;
    bool mipmap_textures_ = false;
    std::string style_name_;
    uint32_t window_width_ = OGTA_DEFAULT_SCREEN_WIDTH;
    uint32_t window_height_ = OGTA_DEFAULT_SCREEN_HEIGHT;
    bool fullscreen_ = false;
    bool vsync_ = false;

#if 0
    bool scale2x_ = true;
#else
    bool scale2x_ = false;
#endif

    // args end

    size_t city_id_ = 0;

#ifdef OGTA_DEFAULT_GRAPHICS_G24
    bool highcolor_data_ = true;
#else
    bool highcolor_data_ = false;
#endif

    // lua config
    float fov_ {};
    float near_plane_ {};
    float far_plane_ {};
    float screen_gamma_ {};
};

} // namespace OpenGTA
