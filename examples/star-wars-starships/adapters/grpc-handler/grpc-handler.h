#pragma once

#include <ports/api.h>
#include <memory>
#include <string>

class GrpcHandler
{
public:
    GrpcHandler(Api& api, const std::string& host, int port);
    virtual ~GrpcHandler();

    void Handle();    
private:    
    Api& _api;
    const std::string _host;
    const int _port;
};
