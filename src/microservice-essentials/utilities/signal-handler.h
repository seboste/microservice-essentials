#pragma once

#include <csignal>
#include <functional>
#include <future>
#include <atomic>

namespace mse
{

/**
    type safe definition of all supported signals that can be handled
*/
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

/**
    Upon receiving one of the above defined system signals, a callback is being invoked. After destruction, the default signal handler is set.

    This class works around the restrictions imposed on signal handlers (e.g. only a very limited set of functions may even be called).
    See https://en.cppreference.com/w/cpp/utility/program/signal for more detauls.

    Note that not all signals may be available on all platforms.

    Note that the current implementation polls for the signal roughly every 5ms. 
    It is safe to assume that the callback is invoked within 20ms after the signal has raised.
 */
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
