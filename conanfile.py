from conan import ConanFile
from conan.tools.cmake import CMake, CMakeToolchain, CMakeDeps, cmake_layout


class ShadowmocapRecipe(ConanFile):
    name = "shadowmocap"
    version = "4.2.0"
    description = "Experimental dev kit client using C++ coroutines for async I/O."
    homepage = "https://github.com/luketokheim/shadowmocap-sdk-cpp"
    license = "BSD"

    # Binary configuration
    settings = "os", "arch", "compiler", "build_type"
    options = {"enable_benchmarks": [True, False]}
    default_options = {"enable_benchmarks": False}

    # Copy sources to when building this recipe for the local cache
    exports_sources = "CMakeLists.txt", "include/*", "src/*", "examples/*", "tests/*", "bench/*"

    def layout(self):
        cmake_layout(self)

    def requirements(self):
        self.requires("asio/1.24.0")

        if self.options.enable_benchmarks:
            self.requires("benchmark/1.7.1")

        if not self.conf.get("tools.build:skip_test", default=False):
            self.requires("catch2/3.3.2")

    def generate(self):
        tc = CMakeToolchain(self)
        tc.generate()
        deps = CMakeDeps(self)
        deps.generate()

    def build(self):
        variables = {"ENABLE_BENCHMARKS": self.options.enable_benchmarks}

        cmake = CMake(self)
        cmake.configure(variables=variables)
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.install()

    def package_info(self):
        self.cpp_info.libs = ["shadowmocap"]
