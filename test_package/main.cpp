#include <microservice-essentials/context.h>
#include <microservice-essentials/utilities/signal-handler.h>
#include <iostream>

int main()
{
    mse::Context ctx;
    std::cout << "context created" << std::endl;

    //SignalHandler requires pthread. Check if it builds.
    mse::SignalHandler signal_handler(mse::Signal::SIG_TERM, [](){});
    std::cout << "signal_handler created" << std::endl;
    return 0;    
}
