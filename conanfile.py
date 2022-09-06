from conans import ConanFile, CMake

class ShadowMocapConan(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    requires = "asio/1.24.0", "catch2/3.1.0"
    generators = "cmake" # "CMakeDeps", "CMakeToolchain"
    default_options = {}

    def build(self):
    	cmake = CMake(self)
    	cmake.configure()
    	cmake.build()
