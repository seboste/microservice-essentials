#include <catch2/catch_test_macros.hpp>
#include <microservice-essentials/reliability/retry-request-hook.h>

using namespace std::chrono_literals;

SCENARIO( "RetryBackoffStrategy", "[reliability][retry][request-hook]" )
{
    GIVEN("a linear retry backoff strategy with 3 retries and a 10ms retry interval")
    {
        mse::LinearRetryBackoff backoff(3, 10ms);

        WHEN("duration until next retry is queried for 1st retry and 0ms total request duration")
        {
            std::optional<mse::RetryBackoffStrategy::Duration> duration = backoff.GetDurationUntilNextRetry(1, 0ms);
            THEN("a further retry is requested")
            {
                REQUIRE(duration.has_value());
                AND_THEN("duration until next retry is 10ms")
                {
                    REQUIRE(duration.value() == 10ms);
                }
            }
        }

        WHEN("duration until next retry is queried for 3rd retry and 0ms total request duration")
        {
            std::optional<mse::RetryBackoffStrategy::Duration> duration = backoff.GetDurationUntilNextRetry(3, 0ms);
            THEN("a further retry is requested")
            {
                REQUIRE(duration.has_value());
                AND_THEN("duration until next retry is 30ms")
                {
                    REQUIRE(duration.value() == 30ms);
                }
            }
        }

        WHEN("duration until next retry is queried for 4th retry and 0ms total request duration")
        {
            std::optional<mse::RetryBackoffStrategy::Duration> duration = backoff.GetDurationUntilNextRetry(4, 0ms);
            THEN("no further retry is requested")
            {
                REQUIRE(!duration.has_value());
            }
        }

        WHEN("duration until next retry is queried for 1st retry and 5ms total request duration")
        {
            std::optional<mse::RetryBackoffStrategy::Duration> duration = backoff.GetDurationUntilNextRetry(1, 5ms);
            THEN("a further retry is requested")
            {
                REQUIRE(duration.has_value());
                AND_THEN("duration until next retry is 5ms")
                {
                    REQUIRE(duration.value() == 5ms);
                }
            }
        }

        WHEN("duration until next retry is queried for 1st retry and 15ms total request duration")
        {
            std::optional<mse::RetryBackoffStrategy::Duration> duration = backoff.GetDurationUntilNextRetry(1, 15ms);
            THEN("a further retry is requested")
            {
                REQUIRE(duration.has_value());
                AND_THEN("duration until next retry is 0ms")
                {
                    REQUIRE(duration.value() == 0ms);
                }
            }
        }

        WHEN("duration until next retry is queried for 1st retry and 31ms total request duration")
        {
            std::optional<mse::RetryBackoffStrategy::Duration> duration = backoff.GetDurationUntilNextRetry(1, 31ms);
            THEN("no further retry is requested")
            {
                REQUIRE(!duration.has_value());                
            }
        }
    }
}

SCENARIO("Retry Request Hook Creation", "[reliability][retry][request-hook]")
{
    WHEN("a retry request hook is created based on the parameters")
    {
        std::unique_ptr<mse::RequestHook> retry_request_hook = mse::RequestHookFactory::GetInstance().Create(
            mse::RetryRequestHook::Parameters(std::make_shared<mse::LinearRetryBackoff>(3, 10ms))
            );
        THEN("the request hook is not null")
        {
            REQUIRE(retry_request_hook != nullptr);
        }
    }
}

SCENARIO("Retry Request Hook", "[reliability][retry][request-hook]")
{
    GIVEN("a retry request hook with linear backoff for 3 attempts")
    {
/*        WHEN()
        std::unique_ptr<mse::RequestHook> retry_request_hook = mse::RequestHookFactory::GetInstance().Create(
            mse::RetryRequestHook::Parameters(std::make_shared<mse::LinearRetryBackoff>(3, 10ms))
            );

*/
    }
}
