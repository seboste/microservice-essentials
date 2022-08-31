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
    class Impl;
    std::unique_ptr<Impl> _impl;    
};
