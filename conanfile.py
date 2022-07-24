from conans import ConanFile, CMake
class MicroserviceEssentials(ConanFile):
    name = "microservice-essentials"
    version = "0.0.1"
    settings = "os", "compiler", "build_type", "arch"
    requires = "catch2/2.13.3"
    generators = "cmake_find_package_multi"
