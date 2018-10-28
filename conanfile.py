from conans import ConanFile, CMake

class GameSystemLibConan(ConanFile):
    name = "AppUpdatingFramework"
    version = "1.0.0"
    license = "<Put the package license here>"
    url = "<Package recipe repository url here, for issues about the package>"
    description = "<Description of AppUpdatingFramework here>"
    settings = "os", "compiler", "build_type", "arch"
    options = {"shared": [True, False]}
    default_options = "shared=False"
    generators = "cmake"
    exports_sources = (
        "CMakeLists.txt",
        "src/*"
    )
    requires = (
        "boost/[>=1.68]@conan/stable",
        "wxwidgets/3.1.1@bincrafters/stable",
        "Poco/[>=1.9]@pocoproject/stable",
        "curlpp/[>=0.8]@sl/testing",
        "OpenSSL/1.0.2p@conan/stable"
    )

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        #self.copy("*.h", dst="include", src="src/Common/include")
        #self.copy("*.h", dst="include", src="src/GameSystem/include")
        self.copy("*.lib", dst="lib", keep_path=False)
        self.copy("*.dll", dst="bin", keep_path=False)
        self.copy("*.so*", dst="lib", keep_path=False)
        self.copy("*.dylib*", dst="lib", keep_path=False)
        self.copy("*.a", dst="lib", keep_path=False)

    #def package_info(self):
    #    self.cpp_info.libs = ["Common", "GameSystem"]

    def imports(self):
        self.copy("*.lib", dst="lib", src="lib")
        self.copy("*.dll", dst="bin", src="bin")
        self.copy("*.so*", dst="lib", src="lib")
        self.copy("*.dylib*", dst="lib", src="lib")
        self.copy("*.a", dst="lib", src="lib")

