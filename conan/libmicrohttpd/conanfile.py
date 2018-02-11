from conans import ConanFile, tools
from conans.tools import os_info, SystemPackageTool


class MicroHTTPDVConan(ConanFile):
    name = "libmicrohttpd"
    version = "0.9.58"
    folder = name.lower()
    settings = "os", "compiler", "build_type", "arch"
    url = "https://gnunet.org/git/libmicrohttpd.git"
    description = "GNU libmicrohttpd is a small C library that is supposed to " \
                  "make it easy to run an HTTP server as part of another application."
    license = "LGPL 2.0"

    def source(self):
        self.run("git clone {}".format(self.url))
        self.run("git checkout tags/v{}".format(self.version), cwd=self.folder)

    def system_requirements(self):
        if os_info.is_linux:
            installer = SystemPackageTool()
            for pkg in ["autoconf", "automake", "texinfo"]:
                installer.install(pkg)

    def build(self):
        self.run("autoreconf -fi", cwd=self.folder)
        self.run("./configure --prefix={}".format(self.package_folder), cwd=self.folder)
        self.run("make -j{}".format(tools.cpu_count()), cwd=self.folder)
        self.run("make install", cwd=self.folder)

    def package(self):
        self.copy("*.a", "lib", "lib", keep_path=False)
        self.copy("*.so*", "lib", "lib", keep_path=False, symlinks=True)
        self.copy("*.h", "include", "include", keep_path=False)

    def package_info(self):
        self.cpp_info.libs.extend(["microhttpd"])
