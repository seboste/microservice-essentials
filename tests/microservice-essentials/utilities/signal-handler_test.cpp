#include <catch2/catch_test_macros.hpp>
#include <microservice-essentials/utilities/signal-handler.h>

using namespace std::chrono_literals;

SCENARIO( "Signal Handler", "[utilities][signal-handler]" )
{    
    GIVEN("A Signal handler attached to SIG_TERM handler")
    {   
        std::atomic<int> sigTermCount = 0;     
        mse::SignalHandler signalHandler(mse::Signal::SIG_TERM, [&](){ sigTermCount++; });        
        WHEN("SIGTERM is raised")
        {
            std::raise(SIGTERM);
            THEN("the callback is called within 10ms")
            {
                std::this_thread::sleep_for(10ms);
                REQUIRE(sigTermCount == 1);
            }
        }

        WHEN("SIGTERM is raised twice with 10ms pause")        
        {
            std::raise(SIGTERM);
            std::this_thread::sleep_for(10ms);
            std::raise(SIGTERM);

            THEN("the callback is called twice within 10ms")
            {
                std::this_thread::sleep_for(10ms);
                REQUIRE(sigTermCount == 2);
            }
        }

        AND_GIVEN("Another signal handler for the SIG_FPE")
        {
            std::atomic<int> sigFpeCount = 0;     
            mse::SignalHandler signalHandler(mse::Signal::SIG_FPE, [&](){ sigFpeCount++; });

            WHEN("SIGTERM is raised")
            {
                std::raise(SIGTERM);
                THEN("the callback is called within 10ms")
                {
                    std::this_thread::sleep_for(10ms);
                    REQUIRE(sigTermCount == 1);
                }
            }
            WHEN("SIGFPE is raised")
            {
                std::raise(SIGFPE);
                THEN("the callback is called within 10ms")
                {
                    std::this_thread::sleep_for(10ms);
                    REQUIRE(sigFpeCount == 1);
                }
            }
        }
        
    }   
}
