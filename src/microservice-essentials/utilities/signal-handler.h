#pragma once

#include <csignal>
#include <functional>
#include <future>
#include <atomic>

namespace mse
{

enum class Signal
{
    SIG_INT = SIGINT, /* Interactive attention signal.  */
    SIG_ILL = SIGILL, /* Illegal instruction.  */
    SIG_ABRT = SIGABRT, /* Abnormal termination.  */
    SIG_FPE = SIGFPE,	/* Erroneous arithmetic operation.  */
    SIG_SEGV = SIGSEGV,	/* Invalid access to storage.  */
    SIG_TERM  =	SIGTERM	/* Termination request.  */
};

class SignalHandler 
{
    public:
        SignalHandler(Signal signal, std::function<void(void)> callback);
        virtual  ~SignalHandler();        
    
    private:
        void callOnSignal();

        Signal _signal = Signal::SIG_TERM;
        std::function<void(void)> _callback;
        std::atomic<bool> _requestCall = false;
        std::atomic<bool> _requestDestruction = false;        
        std::future<void> _callOnSignal;
};



}
