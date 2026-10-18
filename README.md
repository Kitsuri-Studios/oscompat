# oscompat

![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)
![Language: C99](https://img.shields.io/badge/C-C99-blue.svg)
![Platforms: Linux | Windows](https://img.shields.io/badge/platform-Linux%20%7C%20Windows-lightgrey.svg)

Cross-platform portability layer for C. POSIX is the canonical semantics; each
supported platform provides a native backend so the same source builds and
behaves identically on Linux and Windows.

## Introduction

`oscompat` is a compile-time shim. Every platform difference is resolved at
build time through the public headers, and the resulting binary calls the host
API directly. There is no runtime indirection, no emulation process, and no
extra shared library to load.

The public surface is a single umbrella header, `<oscompat.h>`, plus one header
per domain for callers who want to pull in less.

> [!NOTE]
> The library is a static archive. The C compiler sees the platform backend
> directly, so a call such as `os_isatty` compiles down to `isatty` on POSIX and
> `_isatty` on Windows.

## Supported platforms

| Platform | Status |
| --- | --- |
| Linux | Supported |
| Windows | Supported |

Each platform is built and tested on every commit by the CI workflow in
`.github/workflows/ci.yml`.

> [!NOTE]
> macOS is not supported. The vendored WrapDL depends on Linux and ELF
> interfaces (`<link.h>`, `dl_iterate_phdr`) that macOS does not provide.

## Domains

- `string`: portable string and memory helpers missing from some platforms.
- `bytes`: endian-safe integer load and store helpers.
- `path`: path resolution, joining, normalization, and executable discovery.
- `env`: environment variable access and enumeration.
- `process`: subprocess execution, output capture, and exit status.
- `options`: `getopt` and `getopt_long` for platforms without them.
- `file`: whole-file reads and writes, directory creation and removal.
- `dynlib`: dynamic library loading and symbol lookup, backed by WrapDL.
- `terminal`: tty detection, terminal size, and color support.
- `time`: monotonic and wall-clock timing, and sleeping.

## Requirements

- CMake 3.16 or newer.
- A C compiler supporting C99 or newer: GCC, Clang, or MSVC.

## Building

```
cmake -S . -B build
cmake --build build
```

## Installing

```
cmake -S . -B build -DCMAKE_INSTALL_PREFIX=/usr/local
cmake --build build
cmake --install build
```

This installs the static library, the public headers, a CMake package config,
and a pkg-config file.

## Using

With CMake:

```cmake
find_package(oscompat CONFIG REQUIRED)
target_link_libraries(app PRIVATE oscompat::oscompat)
```

With pkg-config:

```
cc app.c $(pkg-config --cflags --libs oscompat)
```

## Testing

```
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

## Portability notes

- POSIX is the reference. Where a platform lacks a primitive such as `memmem`,
  `strlcpy`, `getopt_long`, or recursive directory removal, `oscompat` supplies
  a compatible implementation.
- Path handling uses the host separator. On Windows both `/` and `\` are
  accepted on input.
- `os_getopt` and `os_getopt_long` follow POSIX `getopt` semantics: scanning
  stops at the first non-option and `argv` is never permuted.
- Failures are reported through the return value and `errno`. On Windows,
  `os_set_system_error` maps the Win32 error code to `errno`.

## Layout

```
include/oscompat/   public headers, one per domain
src/<domain>/       implementation, feature-first
tests/<domain>/     tests, one executable per file
cmake/              CMake package config and pkg-config templates
vendor/wrapdl/      vendored WrapDL dynamic linker wrapper
```

## Dependencies

`oscompat` bundles one third-party library for its `dynlib` domain:

- [WrapDL](https://github.com/Kitsuri-Studios/WrapDL) - BSD 3-Clause. Vendored
  under `vendor/wrapdl` at commit `96f7244`.
  - Android linker-namespace code derives from
    [xDL](https://github.com/hexhacking/xDL) - MIT.
  - The namespace bypass approach derives from
    [liblinkernsbypass](https://github.com/bylaws/liblinkernsbypass) -
    BSD 2-Clause.

No other third-party dependencies are bundled. On platforms that require it
for dynamic loading, the build links the system `dl` library through
`CMAKE_DL_LIBS`.

## License

MIT. See LICENSE.
