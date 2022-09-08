#include <catch2/catch_test_macros.hpp>
#include <microservice-essentials/handler.h>
#include <microservice-essentials/cross-cutting-concerns/graceful-shutdown.h>


SCENARIO( "Handler", "[handler][graceful-shutdown]" ) 
{
    GIVEN("a handler")
    {        
        class TestHandler : public mse::Handler
        {
            public:
                bool stopCalled = false;

                TestHandler() : mse::Handler("test-handler") {}

                virtual void Handle() override {}
                virtual void Stop() override { stopCalled = true; }
        };

        {
            TestHandler handler;

            WHEN("Shutdown is requested")
            {
                mse::GracefulShutdown::GetInstance().RequestShutdown();
                THEN("Handler::Stop is being called")
                {
                    REQUIRE(handler.stopCalled == true);
                }
            }
        }

        WHEN("The handler is deleted")
        {
            THEN("creating a new handler is possible")
            {
                REQUIRE_NOTHROW(TestHandler());
            }
        }
    }    
}