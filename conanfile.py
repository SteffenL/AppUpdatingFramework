from conans import ConanFile, CMake

class AufwConan(ConanFile):
    name = "AppUpdatingFramework"
    version = "1.0.0"
    license = "<Put the package license here>"
    url = "<Package recipe repository url here, for issues about the package>"
    description = "<Description of AppUpdatingFramework here>"
    settings = "os", "compiler", "build_type", "arch"
    options = {"shared": [False]}
    default_options = "shared=False"
    generators = "cmake"
    exports_sources = (
        "CMakeLists.txt",
        "src/*",
        "3rd_party/jsoncpp-0.5.0/*",
        "3rd_party/nowide/*"
    )
    requires = (
        "boost/[>=1.66]@conan/stable",
        "wxwidgets/3.1.1@sl/testing",
        "Poco/[>=1.9]@pocoproject/stable",
        "curlpp/[>=0.8]@sl/testing",
        "OpenSSL/1.0.2p@conan/stable"
    )

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        self.copy("*.h", dst="include", src="src/core/include")
        self.copy("*.h", dst="include", src="src/ui/include")
        self.copy("*.h", dst="include", src="src/ui/src/private")
        self.copy("*.lib", dst="lib", keep_path=False)
        self.copy("*.dll", dst="bin", keep_path=False)
        self.copy("*.so*", dst="lib", keep_path=False)
        self.copy("*.dylib*", dst="lib", keep_path=False)
        self.copy("*.a", dst="lib", keep_path=False)

    def package_info(self):
        self.cpp_info.libs = ["aufw_core", "aufw_ui"]
