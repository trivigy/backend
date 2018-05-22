from conans import ConanFile, tools


class AgentConan(ConanFile):
    name = "agent"
    version = "0.0.3"
    folder = name.lower()
    url = "https://github.com/syncaide/agent"
    description = "Distributed web-assembly based mining agent"
    license = "https://gitlab.com/syncaide/agent/blob/master/LICENSE"
    settings = "os", "compiler", "build_type", "arch"
    options = {"source": "ANY"}
    default_options = "source="
    generators = "cmake"

    def source(self):
        if len(self.options.source.value) != 0:
            self.run("cp -R {} .".format(self.options.source.value))
            self.run("make clean", cwd=self.folder)
        else:
            self.run("git clone git@github.com:syncaide/agent.git")
            self.run("git checkout tags/v{0}".format(self.version), cwd=self.folder)

    def build(self):
        self.run("make conan", cwd=self.folder)
        self.run("make build CMAKE_BUILD_TYPE={}".format(self.settings.build_type), cwd=self.folder)

    def package(self):
        self.copy("*.html", "bin", "{}/build/{}/bin".format(self.folder, self.settings.build_type))
        self.copy("*.js", "bin", "{}/build/{}/bin".format(self.folder, self.settings.build_type))
        self.copy("*.wasm", "bin", "{}/build/{}/bin".format(self.folder, self.settings.build_type))

        self.copy("*.proto", "protos", "{}/src/protos".format(self.folder))
        for file in tools.relative_dirs("{}/src/protos".format(self.folder)):
            tools.replace_in_file("{}/src/protos/{}".format(self.folder, file), "option optimize_for = LITE_RUNTIME;", "")
