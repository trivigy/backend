from conans import ConanFile, CMake, tools, AutoToolsBuildEnvironment
from conans.tools import os_info, SystemPackageTool
import os


class ProtobufConan(ConanFile):
    name = "Protobuf"
    version = "3.5.1"
    folder = name.lower()
    shared_lib_version = 10
    url = "https://github.com/google/protobuf"
    description = "Protocol Buffers - Google's data interchange format"
    license = "https://github.com/google/protobuf/blob/master/LICENSE"
    settings = "os", "compiler", "build_type", "arch"
    options = {"shared": [True, False], "fPIC": [True, False], "bin_only": [True, False]}
    default_options = "shared=False", "fPIC=False", "bin_only=False"
    generators = "cmake"

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
            for pkg in ["autoconf", "automake", "libtool", "curl", "make", "g++", "unzip"]:
                installer.install(pkg)

    def build(self):
        if self.settings.os == "Windows":
            args = ["-Dprotobuf_BUILD_TESTS=OFF"]
            if not self.options.bin_only:
                args += ["-DZLIB_ROOT={}".format(dict(self.deps_cpp_info.dependencies)["zlib"].rootpath)]
            args += ["-DBUILD_SHARED_LIBS={}".format('ON' if self.options.shared else 'OFF')]
            if self.settings.compiler == "Visual Studio":
                args += ["-Dprotobuf_MSVC_STATIC_RUNTIME={}".format('ON' if "MT" in str(self.settings.compiler.runtime) else 'OFF')]
            cmake = CMake(self)
            cmake_dir = os.path.sep.join([self.folder, "cmake"])
            self.run("cmake . {} {}".format(cmake.command_line, ' '.join(args)), cwd=cmake_dir)
            self.run("cmake --build . {}".format(cmake.build_config), cwd=cmake_dir)
        else:
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

        if self.settings.os == "Windows":
            if not self.options.bin_only:
                if self.settings.compiler == "Visual Studio":
                    self.copy("*.lib", "lib", "{}/cmake".format(self.folder), keep_path=False)
                elif self.settings.compiler == "gcc":
                    self.copy("*.a", "lib", "{}/cmake".format(self.folder), keep_path=False)

                if self.options.shared:
                    self.copy("*.dll", "bin", "{}/cmake".format(self.folder), keep_path=False)

            self.copy("*.exe", "bin", "{}/cmake".format(self.folder), keep_path=False)
        else:
            if self.settings.os == "Macos":
                if not self.options.shared:
                    self.copy("protoc", "bin", "{}/src/".format(self.folder), keep_path=False)

                    if not self.options.bin_only:
                        self.copy("*.a", "lib", "{}/src/.libs".format(self.folder), keep_path=False)
                else:
                    if not self.options.bin_only:
                        # Change *.dylib dependencies and ids to be relative to @executable_path
                        self.run("bash ../../cmake/change_dylib_names.sh", cwd="{}/src/.libs".format(self.folder))
                        self.copy("*.dylib", "bin", "{}/src/.libs".format(self.folder), keep_path=False, symlinks=True)
                        self.copy("*.dylib", "lib", "{}/src/.libs".format(self.folder), keep_path=False, symlinks=True)

                    # "src/protoc" may be a wrapper shell script which execute "src/.libs/protoc".
                    # Copy "src/.libs/protoc" instead of "src/protoc"
                    self.copy("protoc", "bin", "{}/src/.libs/".format(self.folder), keep_path=False)
            else:
                self.copy("protoc", "bin", "{}/src/".format(self.folder), keep_path=False)

                if not self.options.bin_only:
                    if not self.options.shared:
                        self.copy("*.a", "lib", "{}/src/.libs".format(self.folder), keep_path=False)
                    else:
                        self.copy("*.so*", "lib", "{}/src/.libs".format(self.folder), keep_path=False, symlinks=True)

    def package_info(self):
        if not self.options.bin_only:
            if self.settings.os == "Windows":
                lib_prefix = "lib" if self.settings.compiler == "Visual Studio" else ""
                lib_suffix = "d" if self.settings.build_type == "Debug" else ""
                self.cpp_info.libs = ["%sprotobuf%s" % (lib_prefix, lib_suffix)]
                if self.options.shared:
                    self.cpp_info.defines = ["PROTOBUF_USE_DLLS"]
            elif self.settings.os == "Macos":
                self.cpp_info.libs = ["libprotobuf.a"] if not self.options.shared else ["libprotobuf.{}.dylib".format(self.shared_lib_version)]
            else:
                self.cpp_info.libs = ["protobuf"]
