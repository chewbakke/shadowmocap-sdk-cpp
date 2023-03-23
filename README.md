# ShadowMocap SDK C++

[![test](https://github.com/luketokheim/shadowmocap-sdk-cpp/actions/workflows/test.yml/badge.svg)](https://github.com/luketokheim/shadowmocap-sdk-cpp/actions/workflows/test.yml)

Experimental dev kit client using C++ coroutines.

## Compilers

This project requires C++20 support for coroutines and span container.

- Microsoft Visual Studio 2022
- Clang 13
- G++ 10

## Package managers

This project uses the [Conan](https://conan.io/) C++ package manager to build
for Continuous Integration (CI).

## Build

Install dependencies with the package manager.

```
conan install . --build=missing
```

Use the toolchain file created by the package manager so cmake can locate
libraries with [find_package](https://cmake.org/cmake/help/latest/command/find_package.html).

```console
conan build .
```

Run tests.

```console
cd build/Release
ctest -C Release
```

## License

This project is distributed under a permissive [BSD License](LICENSE).
