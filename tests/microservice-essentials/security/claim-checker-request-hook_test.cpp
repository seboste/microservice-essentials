#include <catch2/catch_test_macros.hpp>
#include <microservice-essentials/security/claim-checker-request-hook.h>

SCENARIO("Claim Checker Request Hook", "[security][authorization]")
{ 
    auto test_func = [](mse::Context&)->mse::Status { return mse::Status::OK; };
    const mse::Status fail_status{mse::StatusCode::permission_denied, "permission denied test"};
    GIVEN("a test predicate")
    {        
        auto test_predicate = [](const std::string& cl)->bool { return cl=="test"; };
    
        WHEN("a claim checker request hool is created based on the parameters")
        {

            std::unique_ptr<mse::RequestHook> claim_checker_request_hook = mse::RequestHookFactory::GetInstance().Create(
                mse::ClaimCheckerRequestHook::Parameters("some_claim", test_predicate, fail_status)
                );
            THEN("the request hook is not null")
            {
                REQUIRE(claim_checker_request_hook != nullptr);
            }

            AND_WHEN("the hook is called without the required claim in the context")
            {
                mse::Context context;
                mse::Status status = claim_checker_request_hook->Process(test_func, context);
                THEN("the fail status is returned")
                {
                    REQUIRE(status == fail_status);
                }
            }
            AND_WHEN("the hook is called with the required claim in the context but doesn't match the predicate")
            {
                mse::Context context { { "some_claim", "invalid"} };
                mse::Status status = claim_checker_request_hook->Process(test_func, context);
                THEN("the fail status is returned")
                {
                    REQUIRE(status == fail_status);
                }
            }
             AND_WHEN("the hook is called with the required claim in the context and matches the predicate")
            {
                mse::Context context { { "some_claim", "test"} };
                mse::Status status = claim_checker_request_hook->Process(test_func, context);
                THEN("the ok status is returned")
                {
                    REQUIRE(status == mse::Status::OK);
                }
            }
        }
    }
}

SCENARIO("Claim Checker Request Predicates", "[security][authorization]")
{
    const mse::Status fail_status{mse::StatusCode::permission_denied, "permission denied test"};

    GIVEN("parameters with a ClaimEqualTo predicate are created")
    {        
        auto parameters = mse::ClaimCheckerRequestHook::ClaimEqualTo("my_claim", "test", fail_status);
        THEN("properties must be correct")
        {
            REQUIRE(parameters.claim == "my_claim");
            REQUIRE(parameters.fail_status == fail_status);
        }
        WHEN("the predicate is evaluated with the required claim")
        {
            bool result = parameters.checker("test");
            THEN("the result is true")
            {
                REQUIRE(result == true);
            }
        }
        WHEN("the predicate is evaluated with some other claim")
        {
            bool result = parameters.checker("bla");
            THEN("the result is false")
            {
                REQUIRE(result == false);
            }
        }
    }

    GIVEN("parameters with a ClaimInSpaceSeparatedList predicate are created")
    {        
        auto parameters = mse::ClaimCheckerRequestHook::ClaimInSpaceSeparatedList("my_claim", "c", fail_status);
        THEN("properties must be correct")
        {
            REQUIRE(parameters.claim == "my_claim");
            REQUIRE(parameters.fail_status == fail_status);
        }
        WHEN("the predicate is evaluated with a required claim")
        {
            bool result = parameters.checker("a b c ");
            THEN("the result is true")
            {
                REQUIRE(result == true);
            }
        }        
        WHEN("the predicate is evaluated with some missing claim")
        {
            bool result = parameters.checker("a b");
            THEN("the result is false")
            {
                REQUIRE(result == false);
            }
        }
    }

    GIVEN("parameters with a ScopeContains predicate are created")
    {        
        auto parameters = mse::ClaimCheckerRequestHook::ScopeContains("c");
        THEN("properties must be correct")
        {
            REQUIRE(parameters.claim == "scope");
            REQUIRE(parameters.fail_status == mse::ClaimCheckerRequestHook::PermissionDenied);
        }
        WHEN("the predicate is evaluated with a required claim")
        {
            bool result = parameters.checker("a b c ");
            THEN("the result is true")
            {
                REQUIRE(result == true);
            }
        }        
        WHEN("the predicate is evaluated with some missing claim")
        {
            bool result = parameters.checker("a b");
            THEN("the result is false")
            {
                REQUIRE(result == false);
            }
        }
    }
}
