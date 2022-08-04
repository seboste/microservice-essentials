#pragma once

#include <ports/api.h>
#include <memory>
#include <string>

namespace httplib { class Server; class Request; class Response; }

class HttpHandler
{
public:
    HttpHandler(Api& api, const std::string& host, int port);
    virtual ~HttpHandler();

    void Handle();    
private:

    void listStarShips(const httplib::Request& request, httplib::Response& response);
    void getStarShip(const httplib::Request& request, httplib::Response& response);
    void updateStatus(const httplib::Request& request, httplib::Response& response);

    Api& _api;
    std::unique_ptr<httplib::Server> _svr;
    const std::string _host;
    const int _port;
};
