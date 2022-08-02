#include "http-handler.h"
#include <httplib/httplib.h>
#include <functional>
#include <iostream>

HttpHandler::HttpHandler(Api& api, const std::string& host, int port)
    : _api(api)
    , _svr(std::make_unique<httplib::Server>())
    , _host(host)
    , _port(port)
{    
    _svr->Get("/StarShips", std::bind(&HttpHandler::listStarShips, this, std::placeholders::_1, std::placeholders::_2));
    _svr->Get("/StarShip", std::bind(&HttpHandler::getStarShip, this, std::placeholders::_1, std::placeholders::_2));
    _svr->Put("/StarShipStatus", httplib::Server::Handler(std::bind(&HttpHandler::updateStatus, this, std::placeholders::_1, std::placeholders::_2)));
}

HttpHandler::~HttpHandler()
{
}

void HttpHandler::Handle()
{
    _svr->listen(_host, _port);
}

void HttpHandler::listStarShips(const httplib::Request& request, httplib::Response& response)
{
    std::cout << "listStarShips" << std::endl;
    std::vector<Starship> starships = _api.ListStarShips();    
}

void HttpHandler::getStarShip(const httplib::Request& request, httplib::Response& response)
{
    std::cout << "getStarShip" << std::endl;
}

void HttpHandler::updateStatus(const httplib::Request& request, httplib::Response& response)
{
    std::cout << "updateStatus" << std::endl;
}
