#include "graceful-shutdown.h"
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

void GracefulShutdown::Shutdown()
{
    _isShutdownRequested = true;
    for(auto cb : _callbacks)
    {
        cb.second();
    }
}
