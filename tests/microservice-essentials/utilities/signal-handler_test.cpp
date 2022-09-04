#include <catch2/catch_test_macros.hpp>
#include <microservice-essentials/utilities/signal-handler.h>

using namespace std::chrono_literals;

SCENARIO( "Signal Handler", "[utilities][signal-handler]" )
{
    GIVEN("A Signal handler attached to SIG_INT handler")
    {   
        std::atomic<int> signalCount = 0;     
        mse::SignalHandler signalHandler(mse::Signal::SIG_INT, [&](){ signalCount++; });        
        WHEN("SIGINT is raised")
        {
            std::raise(SIGINT);
            THEN("the callback is called within 10ms")
            {
                std::this_thread::sleep_for(10ms);
                REQUIRE(signalCount == 1);
            }
        }
    }
    GIVEN("A Signal handler attached to SIG_ABRT handler")
    {   
        std::atomic<int> signalCount = 0;     
        mse::SignalHandler signalHandler(mse::Signal::SIG_ABRT, [&](){ signalCount++; });        
        WHEN("SIGABRT is raised")
        {
            std::raise(SIGABRT);
            THEN("the callback is called within 10ms")
            {
                std::this_thread::sleep_for(10ms);
                REQUIRE(signalCount == 1);
            }
        }
    }
    GIVEN("A Signal handler attached to SIG_FPE handler")
    {   
        std::atomic<int> signalCount = 0;     
        mse::SignalHandler signalHandler(mse::Signal::SIG_FPE, [&](){ signalCount++; });        
        WHEN("SIGFPE is raised")
        {
            std::raise(SIGFPE);
            THEN("the callback is called within 10ms")
            {
                std::this_thread::sleep_for(10ms);
                REQUIRE(signalCount == 1);
            }
        }
    }
    GIVEN("A Signal handler attached to SIG_SEGV handler")
    {   
        std::atomic<int> signalCount = 0;     
        mse::SignalHandler signalHandler(mse::Signal::SIG_SEGV, [&](){ signalCount++; });        
        WHEN("SIGSEGV is raised")
        {
            std::raise(SIGSEGV);
            THEN("the callback is called within 10ms")
            {
                std::this_thread::sleep_for(10ms);
                REQUIRE(signalCount == 1);
            }
        }
    }
#ifndef _WIN32
    GIVEN("A Signal handler attached to SIG_ILL handler")
    {   
        std::atomic<int> signalCount = 0;     
        mse::SignalHandler signalHandler(mse::Signal::SIG_ILL, [&](){ signalCount++; });        
        WHEN("SIGILL is raised")
        {
            std::raise(SIGILL);
            THEN("the callback is called within 10ms")
            {
                std::this_thread::sleep_for(10ms);
                REQUIRE(signalCount == 1);
            }
        }
    }
    GIVEN("A Signal handler attached to SIG_TERM handler")
    {   
        std::atomic<int> signalCount = 0;     
        mse::SignalHandler signalHandler(mse::Signal::SIG_TERM, [&](){ signalCount++; });        
        WHEN("SIGTERM is raised")
        {
            std::raise(SIGTERM);
            THEN("the callback is called within 10ms")
            {
                std::this_thread::sleep_for(10ms);
                REQUIRE(signalCount == 1);
            }
        }
    }
#endif
    return;

    GIVEN("A Signal handler attached to SIG_SHUTDOWN handler")
    {   
        std::atomic<int> sigShutdownCount = 0;     
        mse::SignalHandler signalHandler(mse::Signal::SIG_SHUTDOWN, [&](){ sigShutdownCount++; });        
        WHEN("SIGTERM/SIGINT is raised")
        {
#ifndef _WIN32            
            std::raise(SIGTERM);
#else
            std::raise(SIGINT);
#endif
            THEN("the callback is called within 10ms")
            {
                std::this_thread::sleep_for(10ms);
                REQUIRE(sigShutdownCount == 1);
            }
        }

        WHEN("Shutdown signal is raised twice with 10ms pause")        
        {
            std::raise(static_cast<int>(mse::Signal::SIG_SHUTDOWN));
            std::this_thread::sleep_for(10ms);
            std::raise(static_cast<int>(mse::Signal::SIG_SHUTDOWN));

            THEN("the callback is called twice within 10ms")
            {
                std::this_thread::sleep_for(10ms);
                REQUIRE(sigShutdownCount == 2);
            }
        }

        AND_GIVEN("Another signal handler for the SIG_FPE")
        {
            std::atomic<int> sigFpeCount = 0;     
            mse::SignalHandler signalHandler(mse::Signal::SIG_FPE, [&](){ sigFpeCount++; });

            WHEN("SIGTERM/SIGINT is raised")
            {
                std::raise(static_cast<int>(mse::Signal::SIG_SHUTDOWN));
                THEN("the callback is called within 10ms")
                {
                    std::this_thread::sleep_for(10ms);
                    REQUIRE(sigShutdownCount == 1);
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
