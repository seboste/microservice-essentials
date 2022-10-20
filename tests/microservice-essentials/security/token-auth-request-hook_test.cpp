#include <catch2/catch_test_macros.hpp>
#include <microservice-essentials/security/token-auth-request-hook.h>
#include <microservice-essentials/security/basic-token-auth-request-hook.h>

SCENARIO("Token Authorization Request Hook", "[security][authentication]")
{
    GIVEN("a basic token auth request hook")
    {
        mse::BasicTokenAuthRequestHook hook({"auth", "my-secret-pwd"});
        
        WHEN("the hook is executed with context metadata matching the required metadata")
        {
            mse::Context context({{"auth", "my-secret-pwd"}});
            mse::Status status = hook.Process([](mse::Context&){ return mse::Status::OK;}, context);
            THEN("no error is present")
            {
                REQUIRE(status);
            }
        }

        WHEN("the hook is executed with missing auth metadata")
        {
            mse::Context context;
            mse::Status status = hook.Process([](mse::Context&){ return mse::Status::OK;}, context);
            THEN("unauthenticated is returned")
            {
                REQUIRE(status.code == mse::StatusCode::unauthenticated);
            }
            AND_THEN("status details contain 'missing'")
            {
                REQUIRE(status.details.find("missing") != std::string::npos);
            }
        }

        WHEN("the hook is executed with the wrong secret")
        {
            mse::Context context({{"auth", "wrong-secret"}});
            mse::Status status = hook.Process([](mse::Context&){ return mse::Status::OK;}, context);
            THEN("unauthenticated is returned")
            {
                REQUIRE(status.code == mse::StatusCode::unauthenticated);
            }
            AND_THEN("status details equals 'invalid token'")
            {
                REQUIRE(status.details == "invalid token");
            }
        }
    }
}

SCENARIO("Basic Token Authorization Request Hook", "[security][authentication]")
{    
    WHEN("a basic token auth request hook is created based on the parameters")
    {
        std::unique_ptr<mse::RequestHook> basic_auth_token_request_hook = mse::RequestHookFactory::GetInstance().Create(mse::BasicTokenAuthRequestHook::Parameters({"auth", "my-secret-pwd"}));
        THEN("the request hook is not null")
        {
            REQUIRE(basic_auth_token_request_hook != nullptr);
        }       
    }
}
