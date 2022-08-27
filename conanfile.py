from importlib.metadata import requires
from conans import ConanFile, CMake, tools


class MagicParams(ConanFile):
    name = "magicparams"
    version = "1.0"
    author = "EddyXorb"
    description = "Map enum values to different plain old datatypes and string and check and set and retrieve them without having to remember which parameter maps to which type. "
    settings = "os", "compiler", "build_type", "arch"
    options = {"shared": [True, False], "fPIC": [True, False]}
    default_options = {"shared": False, "fPIC": True}
    generators = "cmake"
    exports_sources = "*"
    requires = "gtest/1.12.1"

    def config_options(self):
        if self.settings.os == "Windows":
            del self.options.fPIC

    def build(self):
        cmake = CMake(self)
        cmake.configure(".")
        cmake.build()

    def package(self):
        self.copy("*.hpp", dst="include", src="include")

    def package_info(self):
        self.cpp_info.libs = ["enummap"]
