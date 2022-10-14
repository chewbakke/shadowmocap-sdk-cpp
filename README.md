# ShadowMocap SDK C++

Experimental dev kit client using C++ coroutines.

## Compilers

This project requires C++20 support for coroutines and span container.

- Microsoft Visual Studio 2022
- Clang 13
- G++ 10

## Package managers

This project uses the [conan](https://conan.io/) C++ package manager to build
for Continuous Integration (CI).

## Build

Create a build folder and install dependencies with the package manager.

```
mkdir build
cd build
conan install .. --build=missing
```

Use the toolchain file created by the package manager so cmake can locate
libraries with [find_package](https://cmake.org/cmake/help/latest/command/find_package.html).

```console
cmake .. \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake \
    -DCMAKE_POLICY_DEFAULT_CMP0091=NEW
cmake --build . --config=Release
```

Run tests.

```console
ctest -C Release
```

## License

This project is distributed under a permissive [BSD License](LICENSE).
