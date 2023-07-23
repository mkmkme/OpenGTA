from conan import ConanFile
from conan.tools.scm import Git
from conan.tools.cmake import CMakeToolchain, CMake, cmake_layout, CMakeDeps


class OpenGTAConan(ConanFile):
    name = "opengta"
    package_type = "application"

    settings = "os", "compiler", "build_type", "arch"

    exports_sources = (
        "cmake",
        "coldet",
        "doc",
        "licenses",
        "lua_addon",
        "math",
        "opensteer",
        "scripts",
        "tests",
        "tools",
        "util",
        "*.cpp",
        "*.h",
        "CMakeLists.txt",
    )

    def set_version(self):
        git = Git(self)
        self.version = (
            git.run("describe").strip().replace("-", "+", 1).replace("-", "_", 1)
        )

    def requirements(self):
        self.requires("cxxopts/[^3.0.0]")
        self.requires("fmt/[^9.1.0]")
        self.requires("lua/[^5.4.4]")
        self.requires("sdl/[^2.26.1]")
        self.requires("physfs/[^3.0.2]")
        self.requires("glu/system")
        self.requires("gtest/[^1.12.1]", test=True)

    def layout(self):
        cmake_layout(self)

    def generate(self):
        deps = CMakeDeps(self)
        deps.generate()

        tc = CMakeToolchain(self)
        tc.user_presets_path = "ConanPresets.json"
        tc.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.install()
