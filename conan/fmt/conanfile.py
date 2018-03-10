from conans import ConanFile, CMake


class FmtConan(ConanFile):
    name = "fmt"
    folder = name.lower()
    version = "4.1.0"
    description = "A modern formatting library"
    license = "https://github.com/fmtlib/fmt/blob/master/LICENSE.rst"
    url = "https://github.com/fmtlib/fmt"
    settings = "os", "arch", "compiler", "build_type"
    options = {"shared": [True, False], "fPIC": [True, False]}
    default_options = "shared=False", "fPIC=True"
    generators = "cmake"

    def config_options(self):
        if self.settings.os == "Windows":
            self.options.remove("fPIC")

    def source(self):
        self.run("git clone https://github.com/fmtlib/fmt")
        self.run("git checkout {}".format(self.version), cwd=self.folder)

    def build(self):
        cmake = CMake(self)
        flags = "-DBUILD_SHARED_LIBS=ON" if self.options.shared else ""
        flags += " -DFMT_TEST=OFF -DFMT_INSTALL=OFF -DFMT_DOCS=OFF"
        if self.settings.os != "Windows" and self.options.fPIC:
            flags += " -DCMAKE_POSITION_INDEPENDENT_CODE=TRUE"
        self.run('cmake fmt {} {}'.format(cmake.command_line, flags))
        self.run("cmake --build . {}".format(cmake.build_config))

    def package(self):
        self.copy("*.h", dst="include/fmt", src="fmt/fmt")
        self.copy("*.h", dst="include/cppformat", src="cppformat")
        self.copy("*.lib", dst="lib", keep_path=False)
        self.copy("*.dll", dst="bin", keep_path=False)
        self.copy("*.so", dst="lib", keep_path=False)
        self.copy("*.a", dst="lib", keep_path=False)

    def package_info(self):
        self.cpp_info.libs.append("fmt")
