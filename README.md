ShadowMocap SDK C++
====

# Build with CMake

Build the test, benchmark, and example apps with CMake.

## Install package manager
```console
git clone https://github.com/Microsoft/vcpkg.git [...]
```

```
mkdir build && cd build
conan install .. --build=missing
```

## Configure

```console
cmake \
    -B build \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_TOOLCHAIN_FILE=[...]/scripts/buildsystems/vcpkg.cmake
```

```console
cmake \
    -B build \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_POLICY_DEFAULT_CMP0091=NEW \
    -DCMAKE_TOOLCHAIN_FILE=build/conan_toolchain.cmake
```

## Build

```console
cmake --build build --config Release
```

## Test

```console
cd build
ctest -C Release
```

# Compiler support

This project requires C++20 support for coroutines and span container.

- Microsoft Visual Studio 2022
- Clang 13
- G++ 10

# License

This project is distributed under a permissive [BSD License](LICENSE).
