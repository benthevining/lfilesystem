# lfilesystem

A C++ filesystem library built on top of std::filesystem

## Features

This library provides object-oriented approaches to working with different kinds of filesystem objects.
Highlights include the `Volume` and `FileWatcher` classes.

## Portability

This library is tested on Mac, Windows, and Linux (with GCC, Clang, and MSVC), as well as cross-compiled
for iOS, tvOS, watchOS, and Emscripten (WebAssembly). All APIs are available on all platforms, with
availability introspection functions for features not supported on all platforms.

## Building

This library is built with CMake. CMake presets are provided for each of the toolchains we target. This
library supports being added to other CMake projects via `find_package()`, `FetchContent`, or a plain
`add_subdirectory()`. In all cases, you should link against the target `limes::lfilesystem` and include
the main header this way:
```cpp
#include <lfilesystem/lfilesystem.h>
```
All of this library's headers can be individually included, but including `lfilesystem.h` is the easiest
way to bring in the entire library.

## Links

[CDash testing dashboard](https://my.cdash.org/index.php?project=lfilesystem)

[Documentation](https://benthevining.github.io/lfilesystem/)
