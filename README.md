# SyncAide

### Build Application
Before you begin make sure to run the following command line in order
to ensure that all of the necessary dependencies are installed.
```bash
make deps/install
```
Once that is done you can build the application by running the next command

```bash
make build CMAKE_BUILD_TYPE={CMAKE_BUILD_TYPE} TARGET={TARGET}
```
* CMAKE_BUILD_TYPE: `Debug`, `Release`, `RelWithDebInfo`, `MinSizeRel` - Default: `Debug`
* TARGET: `all`, `sync`, `syncd` - Default: `all`