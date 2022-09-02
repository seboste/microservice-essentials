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

    void set_shotdown_requested(int)
    {
        shutdown_requested = true;
    }
  

  static_assert( std::atomic<bool>::is_always_lock_free );
}

GracefulShutdownOnSignal::GracefulShutdownOnSignal(int signal)
    : _signal(signal)
{    
    std::signal(signal, set_shotdown_requested);
    _shutdownOnSignal = std::async(&GracefulShutdownOnSignal::waitAndShutdown, this);

}

void GracefulShutdownOnSignal::waitAndShutdown()
{
    using namespace std::chrono_literals;
    std::unique_lock<std::mutex> lk(_mutex);    
    while(_terminationRequested.wait_for(lk, 5ms) == std::cv_status::timeout)
    {
        if(shutdown_requested)
        {
            GracefulShutdown::GetInstance().Shutdown();
            shutdown_requested = false;
        }        
    }
}

GracefulShutdownOnSignal::~GracefulShutdownOnSignal()
{    
    std::signal(_signal, SIG_DFL);    
    _terminationRequested.notify_all();
    _shutdownOnSignal.wait();    
}
