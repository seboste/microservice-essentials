#pragma once

#include <csignal>
#include <functional>
#include <future>
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
        GracefulShutdownOnSignal(int signal = SIGTERM);
        virtual  ~GracefulShutdownOnSignal();

        static void SetShutdownRequested(int); //shouldn't be called accessible for testing purposes
    private:
        void waitAndShutdown();

        int _signal = SIGTERM;
        std::atomic<bool> _requestTermination = false;        
        std::future<void> _shutdownOnSignal;
};



}
