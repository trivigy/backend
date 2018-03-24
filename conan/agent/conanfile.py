from conans import ConanFile


class AideConan(ConanFile):
    name = "agent"
    version = "0.0.1a"
    folder = name.lower()
    url = "https://gitlab.com/syncaide/agent"
    description = "Distributed web-assembly based mining agent"
    license = "https://gitlab.com/syncaide/agent/blob/master/LICENSE"
    settings = "os", "compiler", "build_type", "arch"
    generators = "cmake"

    def source(self):
        self.run("git clone git@gitlab.com:syncaide/agent.git")
        self.run("git checkout -b v{0} origin/v{0}".format(self.version), cwd=self.folder)

    def build(self):
        self.run("make conan", cwd=self.folder)
        self.run("make build CMAKE_BUILD_TYPE={}".format(self.settings.build_type), cwd=self.folder)

    def package(self):
        self.copy("*.js", "bin", "{}/build/{}/bin".format(self.folder, self.settings.build_type))
        self.copy("*.wasm", "bin", "{}/build/{}/bin".format(self.folder, self.settings.build_type))
