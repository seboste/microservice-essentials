#include <catch2/catch_test_macros.hpp>
#include <microservice-essentials/cross-cutting-concerns/error-forwarding-request-hook.h>
#include <utilities/history_logger.h>

SCENARIO("Error Forwarding Request Hook Creation", "[cross-cutting-concerns][error forwarding][request-hook]")
{    
    WHEN("a error forwarding request hook is created based on the parameters")
    {
        std::unique_ptr<mse::RequestHook> error_forwarding_request_hook = mse::RequestHookFactory::GetInstance().Create(mse::ErrorForwardingRequestHook::Parameters());
        THEN("the request hook is not null")
        {
            REQUIRE(error_forwarding_request_hook != nullptr);
        }       
    }
}
