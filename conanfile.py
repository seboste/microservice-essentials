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
        "build_testing": [True, False],
        "build_examples": [True, False]        
    }
    default_options = {
        "build_testing": False,
        "build_examples": False
    }

    def requirements(self):
        if self.options.build_examples:
            self.requires("cpp-httplib/0.11.0")
            self.requires("nlohmann_json/3.11.1")
            self.requires("openssl/3.0.5")
        if self.options.build_testing:
            self.requires("catch2/2.13.9")

    def build(self):
        cmake = CMake(self)
        cmake.definitions['BUILD_TESTING'] = self.options.build_testing
        cmake.definitions['BUILD_EXAMPLES'] = self.options.build_examples
        cmake.configure()
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.install()

    def package_info(self):
        self.cpp_info.libs = ["microservice-essentials"]
    