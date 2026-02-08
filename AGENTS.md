# OpenGTA Developer & Agent Guide

This document serves as a comprehensive guide for AI agents and developers working on the OpenGTA repository. OpenGTA is a modern C++23 reimplementation of the original Grand Theft Auto engine, designed to be cross-platform (Linux, Windows, macOS).

## 1. Project Overview & Architecture

### Source Code Organization (`src/`)
- **`src/core/`**: Core game logic (entities, physics, AI, map management).
  - Key classes: `Pedestrian`, `Car`, `Projectile`, `ActiveMap`.
- **`src/graphics/`**: Rendering pipeline.
  - **Modern (Core Profile)**: Files often suffixed with `_modern` or guarded by `OGTA_USE_MODERN_GL`. Uses shaders (`src/graphics/shader.cpp`).
  - **Legacy (Fixed Function)**: Older implementation details. We are actively migrating away from this.
- **`src/base/`**: Fundamental base classes and definitions.
- **`src/common/`**: Shared code between different modules.
- **`src/data/`**: Handling of raw game data structures.
- **`src/lua-addon/`**: Lua scripting integration.
- **`src/util/`**: Utilities for filesystem (PhysFS), logging, and math helpers.

### Entry Points (`programs/` & `tools/`)
- **`programs/`**: Main executables.
  - `viewer`: **Main game executable/viewer**. This is the primary entry point for the full game engine.
  - `spriteplayer-shader`: Testbed for modern rendering pipeline components (sprites, fonts, shaders).
  - `gfx_extract`: Asset extraction tool.
- **`tools/`**: Auxiliary utilities for debugging and asset inspection.
  - These are used to **test and verify smaller parts of the codebase** in isolation.
  - Contains a mix of modern (`display-font-shader.cpp`) and legacy (`display-font.cpp`) rendering implementations.

## 2. Technology Stack & Dependencies

- **Language**: C++23.
- **Build System**: CMake (Presets available: `default`, `vcpkg`).

### Dependency Management
- **Windows / macOS**: Use `vcpkg` to manage dependencies (SDL2, GLM, PhysFS, fmt, Glad). This ensures consistent versions across platforms.
- **Linux**: Prefer system package managers (e.g., `apt`, `pacman`) for dependencies. `vcpkg` may have compatibility issues on some Linux distributions.

## 3. Configuration & Build Options

Build options are defined in `cmake/build-options.cmake`. Key variables include:

- **`OGTA_USE_MODERN_GL`**: Enable modern OpenGL (3.3 Core) rendering path. This is the primary target for new development.
- **`OGTA_DO_SCALE2X`**: Enable pixel scaling algorithms.
- **`OGTA_WITH_SOUND`**: Enable audio support via SDL_mixer.
- **`OGTA_DEFAULT_SCREEN_WIDTH` / `HEIGHT`**: Default resolution settings.

Refer to `cmake/build-options.cmake` for the full list of configurable macros.

## 4. Modernization Guidelines (OpenGL 3.3+ Core)

The project is transitioning from legacy OpenGL (immediate mode, fixed function) to Modern OpenGL (Core Profile).

- **Macros**: Use `OGTA_USE_MODERN_GL` to distinguish modern paths.
- **Shaders**: Located in `data/shaders/` or embedded. Use `ShaderProgram` class.
- **State Management**: Avoid global state changes where possible; use VAOs/VBOs.
- **Portability**: Do not rely on OS-specific behavior. Code must compile and run on Linux, macOS, and Windows.
- **GLSL**: Target version 330 core (`#version 330 core`).

## 5. Asset Management & Documentation

### Proprietary Formats
OpenGTA relies on original game assets (e.g., `.CMP`, `.STYLE`).
- **Documentation**: The `doc/` directory contains critical notes from the original developer regarding file formats (`sprites.txt`, `slopes1.txt`) and engine behavior. **Consult these files** when working on asset parsing or game logic.
- **PhysFS**: Game data is read via **PhysFS**. **Never** use raw `std::fstream` or C-style `fopen` for game assets. Use the `physfs` wrappers provided in `src/util`.
- **Location**: Assets are expected in a `GTADATA` folder or `gtadata.zip` in the working directory.

## 6. Development Workflow

- **Build**: `cmake --preset vcpkg-debug` (Win/Mac) or `cmake --preset default-debug` (Linux).
- **Run**: `./build/programs/viewer` (example).
- **Conventions**:
  - Indentation: Check existing files (usually 4 spaces or tabs).
  - Naming: Mimic surrounding code (often `snake_case` for variables, `CamelCase` for classes).
  - Logging: Use `log::debug`, `log::info`, `log::warn`, `log::error` functions. These functions are defined in `src/util/log.h`.
    Use `fmt::print` for custom messages.

## 7. Task Specifics for Agents

- **Reading Code**: Always check header files for class definitions.
- **Refactoring**: When porting to modern GL, keep the legacy path available unless instructed to remove it, using the `OGTA_USE_MODERN_GL` define.
- **Testing**: Verify changes by compiling and running the relevant target (usually `viewer` or `spriteplayer-shader`).
