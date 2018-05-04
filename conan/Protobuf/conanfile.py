from conans import ConanFile, tools, AutoToolsBuildEnvironment
from conans.tools import os_info, SystemPackageTool


class ProtobufConan(ConanFile):
    name = "Protobuf"
    version = "3.5.2"
    folder = name.lower()
    url = "https://github.com/google/protobuf"
    description = "Protocol Buffers - Google's data interchange format"
    license = "https://github.com/google/protobuf/blob/master/LICENSE"
    settings = "os", "compiler", "build_type", "arch"
    options = {"shared": [True, False], "fPIC": [True, False], "bin_only": [True, False]}
    default_options = "shared=False", "fPIC=False", "bin_only=False"
    generators = "cmake"
    packages = [
        "autoconf",
        "automake",
        "libtool",
        "curl",
        "make",
        "g++",
        "unzip"
    ]

    def config_options(self):
        if not self.options.bin_only:
            self.options["zlib"].shared = self.options.shared

    def requirements(self):
        if not self.options.bin_only:
            self.requires("zlib/1.2.11@conan/stable")

    def source(self):
        self.run("git clone https://github.com/google/protobuf.git")
        self.run("git checkout tags/v{}".format(self.version), cwd=self.folder)

    def system_requirements(self):
        if os_info.is_linux:
            installer = SystemPackageTool()
            for pkg in self.packages:
                installer.install(pkg)

    def build(self):
        env = AutoToolsBuildEnvironment(self)
        with tools.environment_append(env.vars):
            self.run("./autogen.sh", cwd=self.folder)

            args = ['--disable-dependency-tracking']

            if not self.options.bin_only:
                args += ['--with-zlib']
            if not self.options.shared:
                args += ['--disable-shared']
            if self.options.shared or self.options.fPIC:
                args += ['"CFLAGS=-fPIC" "CXXFLAGS=-fPIC"']

            self.run("./configure {}".format(' '.join(args)), cwd=self.folder)
            self.run("make -j {}".format(tools.cpu_count()), cwd=self.folder)

    def package(self):
        self.copy("*.h", "include", "{}/src".format(self.folder))
        self.copy("*.proto", "include", "{}/src".format(self.folder))
        self.copy("protoc", "bin", "{}/src/".format(self.folder), keep_path=False)

        if not self.options.bin_only:
            if not self.options.shared:
                self.copy("*.a", "lib", "{}/src/.libs".format(self.folder), keep_path=False)
            else:
                self.copy("*.so*", "lib", "{}/src/.libs".format(self.folder), keep_path=False, symlinks=True)

    def package_info(self):
        if not self.options.bin_only:
            self.cpp_info.libs = ["protobuf", "protobuf-lite"]
