#pragma once

#include <memory>
#include <microservice-essentials/utilities/environment.h>
#include <ports/api.h>
#include <string>

class GrpcHandler
{
public:
  GrpcHandler(Api& api, const std::string& host = mse::getenv_or("HOST", "0.0.0.0"),
              int port = mse::getenv_or("PORT", 50051));
  virtual ~GrpcHandler();

  void Handle();

private:
  class Impl;
  std::unique_ptr<Impl> _impl;
};
