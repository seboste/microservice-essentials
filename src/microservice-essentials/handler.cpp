#include "handler.h"
#include <microservice-essentials/cross-cutting-concerns/graceful-shutdown.h>

using namespace mse;

Handler::Handler(const std::string& name)
    : _name(name)
{
    GracefulShutdown::GetInstance().Register(_name, [this]() { Stop(); });
}

Handler::~Handler()
{
    GracefulShutdown::GetInstance().UnRegister(_name);
}
