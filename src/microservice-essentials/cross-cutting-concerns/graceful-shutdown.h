#pragma once

#include <microservice-essentials/utilities/signal-handler.h>
#include <atomic>
#include <functional>
#include <unordered_map>

namespace mse
{

class GracefulShutdown
{
    public:
        static GracefulShutdown& GetInstance();        

        void Register(const std::string& id, std::function<void(void)> shutdown_callback);
        void UnRegister(const std::string& id);        

        void Shutdown();
        bool IsShutdownRequested() const;
        
    protected:
        GracefulShutdown();
        virtual ~GracefulShutdown();        

    private:

        std::unordered_map<std::string, std::function<void(void)>> _callbacks;                
        std::atomic<bool> _isShutdownRequested = false; //use atomic because Shutdown() and IsShutdownRequested() may be called from different threads
};


//should be instantiated AFTER all callbacks have been registered to the GracefulShutdown
class GracefulShutdownOnSignal
{
    public:
        GracefulShutdownOnSignal(Signal signal = Signal::SIG_TERM)
            : _signalHandler(signal, [](){ GracefulShutdown::GetInstance().Shutdown(); })
        {}
        virtual  ~GracefulShutdownOnSignal() {}        
    private:
        SignalHandler _signalHandler;
};



}
