from conans import ConanFile


class FifoMapConan(ConanFile):
    name = "fifo_map"
    version = "1.0"
    url = 'https://github.com/nlohmann/fifo_map'
    description = 'a FIFO-ordered associative container for C++'
    license = "https://github.com/nlohmann/fifo_map/blob/master/LICENSE.MIT"
    exports_sources = "include/*"
    no_copy_source = True
    generators = "cmake"

    def source(self):
        self.run("git clone https://github.com/nlohmann/fifo_map.git")

    def package(self):
        self.copy("*.hpp", "include/nlohmann", "{}/src".format(self.name))
