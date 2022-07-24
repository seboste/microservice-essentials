from conans import ConanFile, CMake
class MicroserviceEssentials(ConanFile):
    name = "microservice-essentials"
    version = "0.0.1"
    url = "https://github.com/seboste/microservice-essentials"
    license = "MIT"
    author = "Sebastian Steger (Sebastian.Steger@gmail.com)"
    description = """microservice-essentials is a portable, independent C++ library that takes care of typical recurring concerns that occur in microservice development."""
    settings = "os", "compiler", "build_type", "arch"    
    generators = "cmake_find_package_multi"
    scm = {
        "type": "git",        
        "url": "https://github.com/seboste/microservice-essentials.git",
        "revision": "auto"
    }
    options = {
        "build_tests": [True, False],
        "build_examples": [True, False]        
    }
    default_options = {
        "build_tests": False,
        "build_examples": False
    }

    def requirements(self):
        if self.options.build_tests:
            self.requires("catch2/2.13.3")

    def build(self):
        cmake = CMake(self)
        cmake.definitions['BUILD_TESTS'] = self.options.build_tests
        cmake.definitions['BUILD_EXAMPLES'] = self.options.build_examples
        cmake.configure()
        cmake.build()

    def package(self):
        self.copy("*.h", dst="include", src="src")
        self.copy("*.lib", dst="lib", keep_path=False)
        self.copy("*.dll", dst="bin", keep_path=False)
        self.copy("*.dylib*", dst="lib", keep_path=False)
        self.copy("*.so", dst="lib", keep_path=False)
        self.copy("*.a", dst="lib", keep_path=False)

    def package_info(self):
        self.cpp_info.libs = ["microservice-essentials"]
    