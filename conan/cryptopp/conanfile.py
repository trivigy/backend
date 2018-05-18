from conans import ConanFile, AutoToolsBuildEnvironment
from conans import tools


class CryptoppConan(ConanFile):
    name = "cryptopp"
    version = "7.0.0"
    folder = name.lower()
    url = "https://github.com/weidai11/cryptopp.git"
    description = "free C++ class library of cryptographic schemes."
    settings = "os", "compiler", "build_type", "arch"
    license = "https://github.com/weidai11/cryptopp/blob/master/License.txt"
    options = {"shared": [True, False], "tests": [True, False]}
    default_options = "shared=False", "tests=False"

    def source(self):
        self.run("git clone {}".format(self.url))
        self.run("git checkout tags/{name}_{version}".format(
            name=self.name.upper(),
            version=self.version.replace(".", "_")
        ), cwd=self.folder)

    def build(self):
        env = AutoToolsBuildEnvironment(self)
        with tools.environment_append(env.vars):
            if self.options.shared:
                self.run('make dynamic', cwd=self.folder)
            else:
                self.run('make static', cwd=self.folder)

            if self.options.tests:
                self.run('make test check', cwd=self.folder)

    def package(self):
        self.copy(pattern="*.h", dst="include/cryptopp", src=".")
        self.copy(pattern="*.so", dst="lib", src=".", keep_path=False)
        self.copy(pattern="*.a", dst="lib", src=".", keep_path=False)

    def package_info(self):
        self.cpp_info.libs = ["cryptopp"]
