#pragma once

#include <memory>
#include <microservice-essentials/handler.h>
#include <microservice-essentials/utilities/environment.h>
#include <ports/api.h>
#include <string>

namespace httplib
{
class Server;
class Request;
class Response;
} // namespace httplib

namespace mse
{
class Cache;
} //  namespace mse

class HttpHandler : public mse::Handler
{
public:
  HttpHandler(Api& api, const std::string& host = mse::getenv_or("HOST", "0.0.0.0"),
              int port = mse::getenv_or("PORT", 8080));
  virtual ~HttpHandler();

  virtual void Handle() override;
  virtual void Stop() override;

private:
  void listStarShips(const httplib::Request& request, httplib::Response& response);
  void getStarShip(const httplib::Request& request, httplib::Response& response);
  void updateStatus(const httplib::Request& request, httplib::Response& response);

  Api& _api;
  std::unique_ptr<httplib::Server> _svr;
  const std::string _host;
  const int _port;
  std::shared_ptr<mse::Cache> _cache;
};
