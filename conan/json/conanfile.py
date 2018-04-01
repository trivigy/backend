from conans import ConanFile


class JsonConan(ConanFile):
    name = "json"
    version = "3.1.2"
    folder = name.lower()
    url = "https://github.com/nlohmann/json"
    description = "JSON for Modern C++"
    license = "https://github.com/nlohmann/json/blob/develop/LICENSE.MIT"
    exports_sources = "include/*"
    no_copy_source = True
    generators = "cmake"

    def source(self):
        self.run("git clone https://github.com/nlohmann/json.git")
        self.run("git checkout tags/v{}".format(self.version), cwd=self.folder)

    def package(self):
        self.copy("*.hpp", "include", "{}/include".format(self.folder))
