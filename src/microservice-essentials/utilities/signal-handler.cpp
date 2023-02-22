#include "signal-handler.h"
#include <chrono>
#include <exception>
#include <thread>

using namespace mse;

//required to use it in a signal handler



namespace
{

template<Signal S>
struct SignalCallbackHolder
{
    static_assert( std::atomic<bool>::is_always_lock_free );
    static std::atomic<bool> signalRaised;
    static void RaiseSignal(int) { SignalCallbackHolder<S>::signalRaised = true; }
};

template<Signal S>
std::atomic<bool> SignalCallbackHolder<S>::signalRaised = false;

typedef void (*signal_callback_ptr)(int);
signal_callback_ptr getCallback(Signal signal)
{
    switch(signal)
    {
        case Signal::SIG_INT:   return SignalCallbackHolder<Signal::SIG_INT>::RaiseSignal;        
        case Signal::SIG_ABRT:  return SignalCallbackHolder<Signal::SIG_ABRT>::RaiseSignal;
        case Signal::SIG_FPE:   return SignalCallbackHolder<Signal::SIG_FPE>::RaiseSignal;
        case Signal::SIG_SEGV:  return SignalCallbackHolder<Signal::SIG_SEGV>::RaiseSignal;
#ifndef _WIN32
        case Signal::SIG_ILL:   return SignalCallbackHolder<Signal::SIG_ILL>::RaiseSignal;
        case Signal::SIG_TERM:  return SignalCallbackHolder<Signal::SIG_TERM>::RaiseSignal;
#endif
        default: throw std::out_of_range("unknown signal");
    }
}

std::atomic<bool>& getSignalRaised(Signal signal)
{
    switch(signal)
    {
        case Signal::SIG_INT:   return SignalCallbackHolder<Signal::SIG_INT>::signalRaised;
        case Signal::SIG_ABRT:  return SignalCallbackHolder<Signal::SIG_ABRT>::signalRaised;
        case Signal::SIG_FPE:   return SignalCallbackHolder<Signal::SIG_FPE>::signalRaised;
        case Signal::SIG_SEGV:  return SignalCallbackHolder<Signal::SIG_SEGV>::signalRaised;
#ifndef _WIN32
        case Signal::SIG_ILL:   return SignalCallbackHolder<Signal::SIG_ILL>::signalRaised;        
        case Signal::SIG_TERM:  return SignalCallbackHolder<Signal::SIG_TERM>::signalRaised;
#endif
        default: throw std::out_of_range("unknown signal");
    }
}

}


SignalHandler::SignalHandler(Signal signal, std::function<void(void)> callback)
    : _signal(signal)
    , _callback(callback)
{    
    std::signal(static_cast<int>(signal), getCallback(signal));
    _callOnSignal = std::async(&SignalHandler::callOnSignal, this);
    
    using namespace std::chrono_literals;
    std::this_thread::sleep_for(5ms);
}

void SignalHandler::callOnSignal()
{
    std::atomic<bool>& signalRaised = getSignalRaised(_signal);
    using namespace std::chrono_literals;    
    while(_requestDestruction == false)
    {
        if(signalRaised)
        {
            _callback();
            signalRaised = false;
        }
        std::this_thread::sleep_for(5ms);
    }
}

SignalHandler::~SignalHandler()
{    
    std::signal(static_cast<int>(_signal), SIG_DFL);    
    _requestDestruction = true;    
}