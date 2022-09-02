#include <catch2/catch_test_macros.hpp>
#include <microservice-essentials/cross-cutting-concerns/graceful-shutdown.h>

using namespace std::chrono_literals;

SCENARIO( "Graceful Shutdown Registration & Callback Invocation", "[graceful-shutdown]" )
{ 
    GIVEN("Graceful Shutdown with one callback registered")
    {
        std::atomic<int> cbCallCount = 0;
        mse::GracefulShutdown::GetInstance().Register("test_cb", [&](){ cbCallCount++; } );
        
        WHEN("Shutdown is invoked")
        {
            mse::GracefulShutdown::GetInstance().Shutdown();
            THEN("the callback is called")
            {
                REQUIRE(cbCallCount == 1);
            }
        }
        
        WHEN("a second callback is registered and shutdown is invoked")
        {    
            bool hasBeenCalled2 = false;
            mse::GracefulShutdown::GetInstance().Register("test2_cb", [&](){ hasBeenCalled2 = true;} );
            mse::GracefulShutdown::GetInstance().Shutdown();
            THEN("both callbacks are called")
            {
                REQUIRE(cbCallCount == 1);
                REQUIRE(hasBeenCalled2 == true);
            }            
            mse::GracefulShutdown::GetInstance().UnRegister("test2_cb");
        }

        WHEN("the callback is unregistered and shutdown is invoked")
        {
            mse::GracefulShutdown::GetInstance().UnRegister("test_cb");    
            mse::GracefulShutdown::GetInstance().Shutdown();
           THEN("the callback is not called anymore")
            {
                REQUIRE(cbCallCount == 0);
            }
        }

        WHEN("the a callback is registered with the same id")
        {
            THEN("a runtime error occurs")
            {
                REQUIRE_THROWS_AS(mse::GracefulShutdown::GetInstance().Register("test_cb", [](){}), std::runtime_error );
            }
        }

        AND_GIVEN("a GracefulShutdownOnSignal instance connected to the SIGTERM signal")
        {
            {                
                mse::GracefulShutdownOnSignal gracefulShutdown;
                
                WHEN("the sigterm signal is raised")
                {                    
                    //std::raise(SIGTERM);
                    mse::GracefulShutdownOnSignal::SetShutdownRequested(1);
                    THEN("the callback is called within 10ms")
                    {                        
                        std::this_thread::sleep_for(10ms);
                        REQUIRE(cbCallCount == 1);
                    }
                }

                WHEN("the sigterm signal is raised twice")
                {                
                    //std::raise(SIGTERM);
                    mse::GracefulShutdownOnSignal::SetShutdownRequested(1);
                    std::this_thread::sleep_for(10ms);
                    //std::raise(SIGTERM);
                    mse::GracefulShutdownOnSignal::SetShutdownRequested(1);

                    THEN("the callback is called within 10ms")
                    {                        
                        std::this_thread::sleep_for(10ms);
                        REQUIRE(cbCallCount == 2);
                    }
                }
            }

            WHEN("the instance has been deleted and the sigterm signal is raised")
            {
                //std::raise(SIGTERM);
                mse::GracefulShutdownOnSignal::SetShutdownRequested(1);
                THEN("the callback is NOT called within 10ms")
                {                    
                    std::this_thread::sleep_for(10ms);
                    REQUIRE(cbCallCount == 0);
                }
            }
        }

        mse::GracefulShutdown::GetInstance().UnRegister("test_cb");
    }   
}
