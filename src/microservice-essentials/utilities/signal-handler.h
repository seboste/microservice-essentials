#pragma once

#include <csignal>
#include <functional>
#include <future>
#include <atomic>

namespace mse
{

//type safe definition of all supported signals that can be handled
enum class Signal
{
    SIG_INT = SIGINT, /* Interactive attention signal.  */    
    SIG_ABRT = SIGABRT, /* Abnormal termination.  */
    SIG_FPE = SIGFPE,	/* Erroneous arithmetic operation.  */
    SIG_SEGV = SIGSEGV,	/* Invalid access to storage.  */
#ifndef _WIN32 //the following signals are not supported in windows
    SIG_ILL = SIGILL, /* Illegal instruction.  */
    SIG_TERM  =	SIGTERM,	/* Termination request.  */
    SIG_SHUTDOWN = SIGTERM      //special platform independent signal that maps to SIGTERM on non windows systems...
#else
    SIG_SHUTDOWN = SIGINT       //...and to SIGINT on windows systems
#endif
};

class SignalHandler 
{
    public:
        SignalHandler(Signal signal, std::function<void(void)> callback);
        virtual  ~SignalHandler();        
    
    private:
        void callOnSignal();

        Signal _signal = Signal::SIG_SHUTDOWN;
        std::function<void(void)> _callback;
        std::atomic<bool> _requestCall = false;
        std::atomic<bool> _requestDestruction = false;        
        std::future<void> _callOnSignal;
};



}
