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

namespace
{
    static std::atomic<bool> shutdown_requested = false;
    static_assert( std::atomic<bool>::is_always_lock_free );
}

void GracefulShutdownOnSignal::SetShutdownRequested(int)
{
    shutdown_requested = true;
}

GracefulShutdownOnSignal::GracefulShutdownOnSignal(int signal)
    : _signal(signal)
{    
    std::signal(signal, GracefulShutdownOnSignal::SetShutdownRequested);
    _shutdownOnSignal = std::async(&GracefulShutdownOnSignal::waitAndShutdown, this);
    
    using namespace std::chrono_literals;
    std::this_thread::sleep_for(5ms);
}

void GracefulShutdownOnSignal::waitAndShutdown()
{
    using namespace std::chrono_literals;    
    while(_requestTermination == false)
    {
        if(shutdown_requested)
        {
            GracefulShutdown::GetInstance().Shutdown();
            shutdown_requested = false;
        }
        std::this_thread::sleep_for(5ms);
    }
}

GracefulShutdownOnSignal::~GracefulShutdownOnSignal()
{    
    std::signal(_signal, SIG_DFL);
    _requestTermination = true;    
}
