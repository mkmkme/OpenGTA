Change Log
==========

All relevant changes are documented in this file. This file and the deploy
action related to that is based on the files in troglobit's [finit].


[0.5.1][] - 2023-02-09
----------------------

### Fixes
* Fixed Lua invocation in the tools. `luaviewer` and `lua_map_test` can now be
  compiled and can use the scripts from `scripts` directory.


[0.5.0][] - 2023-02-08
----------------------

This release mostly addresses introduction of Github actions (including the one
that automatically creates a release for a tag). Additionally, it contains some
breaking changes to previous version so the minor version had to be bumped.

### Changes
* `g24` is renamed to `g24-viewer` and uses SDL2
* Some tools are detached from hackish `main2.cpp` file

### Fixes
* Make `display-font` work


[0.4.0][] - 2023-01-30
----------------------

### Changes
* Use Conan to handle dependencies
* Switch to SDL2 (The project used SDL1.2 previously)
* Use `fmt` library for formatting strings and in logging
* Reduce heap usage more
* Multiple refactorings
* Removed some obsolete tools


[0.3.0][] - 2021-06-03
----------------------

### Changes
* Switched to cmake


[0.2.3][] - 2021-06-03
----------------------

### Fixes
* Fixed `objdump` compilation


[0.2.2][] - 2021-06-02
----------------------

### Changes
* Use log more instead of plain `stderr`
* Replace some `assert`'s with exceptions
* Reduce heap usage

### Fixes
* Make `luaviewer` compile. It can't run anything yet but at least doesn't cause
  compilation errors


[0.2.1][] - 2021-05-21
----------------------

### Changes
* Enable colors in log messages
* Added some more logging
* Multiple refactorings

### Fixes
* Make `sound_test` build
* Fixed `PHYSFS` deprecation warning
* Fixed some warnings raised by Valgrind


[0.2.0][] - 2021-05-18
----------------------

### Changes
* Removed Loki dependency


[0.1.1][] - 2021-02-18
----------------------

### Changes
* Added `.gitignore` to the repo

### Fixes
* Fixed build on modern Linux and macOS


0.1.0 - 2015-12-03
------------------

* The last commit in [madebr] repository. The code itself is marked with a date
of 2007-06-14, so there's a good chance that this is the actual time of that
revision.

[UNRELEASED]: https://github.com/mkmkme/OpenGTA/compare/0.5.1...HEAD
[0.5.1]: https://github.com/mkmkme/OpenGTA/compare/0.5.0...0.5.1
[0.5.0]: https://github.com/mkmkme/OpenGTA/compare/0.4.0...0.5.0
[0.4.0]: https://github.com/mkmkme/OpenGTA/compare/0.3.0...0.4.0
[0.3.0]: https://github.com/mkmkme/OpenGTA/compare/0.2.3...0.3.0
[0.2.3]: https://github.com/mkmkme/OpenGTA/compare/0.2.2...0.2.3
[0.2.2]: https://github.com/mkmkme/OpenGTA/compare/0.2.1...0.2.2
[0.2.1]: https://github.com/mkmkme/OpenGTA/compare/0.2.0...0.2.1
[0.2.0]: https://github.com/mkmkme/OpenGTA/compare/0.1.1...0.2.0
[0.1.1]: https://github.com/mkmkme/OpenGTA/compare/0.1.0...0.1.1
[finit]: https://github.com/troglobit/finit
[madebr]: https://github.com/madebr/OpenGTA