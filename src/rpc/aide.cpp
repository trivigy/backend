#include "aide.h"

rpc::AideService::AideService(AbstractServerConnector &connector) :
    stubs::aide::AbstractStubServer(connector) {
}

string rpc::AideService::submit(const string &name) {
    return "Hello " + name;
}