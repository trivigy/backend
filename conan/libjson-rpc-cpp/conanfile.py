from conans import ConanFile, CMake, tools
from conans.tools import os_info, SystemPackageTool


class LibJsonRpcCppConan(ConanFile):
    name = "libjson-rpc-cpp"
    version = "1.1.0"
    folder = name.lower()
    url = "https://github.com/cinemast/libjson-rpc-cpp"
    description = "C++ framework for json-rpc (json remote procedure call)"
    license = "https://github.com/cinemast/libjson-rpc-cpp/blob/master/LICENSE.txt"
    requires = "jsoncpp/1.8.4@theirix/stable", "libcurl/7.56.1@bincrafters/stable"
    settings = "os", "compiler", "build_type", "arch"
    options = {"shared": [True, False]}
    default_options = "shared=False"
    generators = "cmake"

    def source(self):
        self.run("git clone https://github.com/cinemast/libjson-rpc-cpp.git")
        self.run("git checkout tags/v{}".format(self.version), cwd=self.folder)

        cmake_name = "{}/src/stubgenerator/CMakeLists.txt".format(self.folder)
        tools.replace_in_file(cmake_name, '''if (BUILD_STATIC_LIBS AND NOT BUILD_SHARED_LIBS)
        target_link_libraries(jsonrpcstub jsonrpccommonStatic libjsonrpcstubStatic )
else()
        target_link_libraries(jsonrpcstub jsonrpccommon libjsonrpcstub )
endif()''', 'target_link_libraries(jsonrpcstub jsonrpccommon libjsonrpcstub)')

    def system_requirements(self):
        if os_info.is_linux:
            installer = SystemPackageTool()
            for pkg in ["libcurl4-openssl-dev", "libjsoncpp-dev", "libargtable2-dev", "libmicrohttpd-dev", "libhiredis-dev"]:
                installer.install(pkg)

    def build(self):
        cmake = CMake(self)
        cmake.definitions["COMPILE_TESTS"] = False

        if not self.options.shared:
            cmake.definitions["BUILD_STATIC_LIBS"] = True

        cmake.configure(source_folder="{}/{}".format(self.source_folder, self.folder))
        cmake.build()

    def package(self):
        self.copy("*.h", "include", "{}/src".format(self.folder))
        self.copy("*.h", "include/jsonrpccpp", "gen/jsonrpccpp")
        self.copy("*", "bin", "bin", keep_path=False)
        self.copy("*.a", "lib", "lib", keep_path=False, symlinks=True)
        self.copy("*.so*", "lib", "lib", keep_path=False, symlinks=True)

    def package_info(self):
        self.cpp_info.libs = tools.collect_libs(self)
