from conans import ConanFile, CMake

class Vkatest1Conan(ConanFile):
    name = "vkaTest1"
    version = "0.0.1"
    license = "<Put the package license here>"
    author = "<Put your name here> <And your email here>"
    description = "<Description of Vkatest1 here>"
    settings = "os", "compiler", "build_type", "arch", "cppstd"
    options = {"shared": [True, False]}
    default_options = "shared=False"
    generators = "cmake"
    exports_sources = "!build/*", "*"
    requires = "tinygltf/2.0@jeffw387/testing", "filesystem/X.Y.Z@jeffw387/testing", "vkaEngine/0.0.1@jeffw387/testing", "Catch2/2.5.0@catchorg/stable"

    def build(self):
        cmake = CMake(self)
        cmake.verbose = True
        cmake.configure()
        cmake.build()

    def package(self):
        self.copy("*.hpp", dst="include", src="src")
        self.copy("*.lib", dst="lib", keep_path=False)
        self.copy("*.dll", dst="bin", keep_path=False)
        self.copy("*.dylib*", dst="lib", keep_path=False)
        self.copy("*.so", dst="lib", keep_path=False)
        self.copy("*.a", dst="lib", keep_path=False)