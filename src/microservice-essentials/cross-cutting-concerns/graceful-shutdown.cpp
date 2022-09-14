#include "graceful-shutdown.h"
#include <microservice-essentials/observability/logger.h>
#include <chrono>

using namespace mse;

GracefulShutdown::GracefulShutdown()
{
}

GracefulShutdown::~GracefulShutdown()
{
}

GracefulShutdown& GracefulShutdown::GetInstance()
{
    static GracefulShutdown _instance;
    return _instance;
}

void GracefulShutdown::Register(const std::string& id, std::function<void(void)> shutdown_callback)
{
    if(!_callbacks.insert({id, shutdown_callback}).second)
    {
        throw std::runtime_error(std::string("unable to register shutdown callback for ") + id + " : id already exists.");
    }
}

void GracefulShutdown::UnRegister(const std::string& id)
{
    _callbacks.erase(id);
}


bool GracefulShutdown::IsShutdownRequested() const
{
    return _isShutdownRequested;
}

void GracefulShutdown::RequestShutdown()
{
    LogProvider::GetLogger().Write(LogLevel::info, "shutting down service...");
    _isShutdownRequested = true;
    for(auto cb : _callbacks)
    {
        LogProvider::GetLogger().Write(LogLevel::trace, std::string("informing ") + cb.first + " about shutdown.");
        cb.second();
    }
    LogProvider::GetLogger().Write(LogLevel::info, "...all registered components informed about shutdown.");
}
