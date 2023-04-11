#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <microservice-essentials/reliability/retry-request-hook.h>
#include <cmath>

using namespace std::chrono_literals;

SCENARIO( "RetryBackoffStrategy", "[reliability][retry][request-hook]" )
{
    GIVEN("a linear retry backoff strategy with 3 retries and a 10ms retry interval")
    {
        std::shared_ptr<mse::LinearRetryBackoff> backoff = std::make_shared<mse::LinearRetryBackoff>(3, 10ms);

        WHEN("duration until next retry is queried for 1st retry and 0ms total request duration")
        {
            std::optional<mse::RetryBackoffStrategy::Duration> duration = backoff->GetDurationUntilNextRetry(1, 0ms);
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
            std::optional<mse::RetryBackoffStrategy::Duration> duration = backoff->GetDurationUntilNextRetry(3, 0ms);
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
            std::optional<mse::RetryBackoffStrategy::Duration> duration = backoff->GetDurationUntilNextRetry(4, 0ms);
            THEN("no further retry is requested")
            {
                REQUIRE(!duration.has_value());
            }
        }

        WHEN("duration until next retry is queried for 1st retry and 5ms total request duration")
        {
            std::optional<mse::RetryBackoffStrategy::Duration> duration = backoff->GetDurationUntilNextRetry(1, 5ms);
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
            std::optional<mse::RetryBackoffStrategy::Duration> duration = backoff->GetDurationUntilNextRetry(1, 15ms);
            THEN("a further retry is requested")
            {
                REQUIRE(duration.has_value());
                AND_THEN("duration until next retry is 0ms")
                {
                    REQUIRE(duration.value() == 0ms);
                }
            }
        }

        AND_GIVEN("a gaussian jitter decorator on that linear backoff strategy with a sigma of 2ms")
        {
            mse::BackoffGaussianJitterDecorator backoff_with_jitter(backoff, 2ms);

            WHEN("1000 queries for 1st retry and 0ms total request duration are issued")
            {
                mse::RetryBackoffStrategy::Duration averageDuration = 0ms;
                double averageRMS = 0.0;
                for(int i=0;i<1000; ++i)
                {
                    std::optional<mse::RetryBackoffStrategy::Duration> duration = backoff_with_jitter.GetDurationUntilNextRetry(1, 0ms);
                    averageDuration += duration.value();
                    averageRMS += (duration.value() - 10ms).count() * (duration.value() - 10ms).count();
                }

                averageDuration /= 1000.0;
                averageRMS = std::sqrt(averageRMS / 1000.0);

                THEN("the average duration is roughly 10ms")
                {
                    REQUIRE(averageDuration.count() == Catch::Approx((10ms).count()).epsilon(0.01));
                }
                THEN("the standard deviation is roughly 2ms")
                {
                    REQUIRE(averageRMS == Catch::Approx((2ms).count()).epsilon(0.1));
                }
            }
        }
    }

    GIVEN("an exponential retry backoff strategy with 3 retries and a 10ms first interval and a base of 2")
    {
        mse::ExponentialRetryBackoff backoff(3, 10ms, 2.0f);

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
                AND_THEN("duration until next retry is 70ms (10+20+40ms)")
                {
                    REQUIRE(duration.value() == 70ms);
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
    GIVEN("a retry request hook with linear backoff for 3 attempts for unavailable error code")
    {
        std::unique_ptr<mse::RequestHook> retry_request_hook = mse::RequestHookFactory::GetInstance().Create(
            mse::RetryRequestHook::Parameters(std::make_shared<mse::LinearRetryBackoff>(3, 10ms), { mse::StatusCode::unavailable })
            );

        WHEN("it processes a succeeding function")
        {
            int call_count = 0;
            mse::Context ctx;
            mse::Status status = retry_request_hook->Process([&](mse::Context&) {
                ++call_count;
                return mse::Status::OK;
            }, ctx);

            THEN("status is ok")
            {
                REQUIRE(status == mse::Status::OK);
            }
            AND_THEN("function has been called exactly once")
            {
                REQUIRE(call_count == 1);
            }
        }

        WHEN("it processes a function returning an internal error")
        {
            int call_count = 0;
            mse::Context ctx;
            mse::Status status = retry_request_hook->Process([&](mse::Context&) {
                ++call_count;
                return mse::Status{ mse::StatusCode::internal, ""};
            }, ctx);

            THEN("status is internal error")
            {
                REQUIRE(status.code == mse::StatusCode::internal);
            }
            AND_THEN("function has been called exactly once")
            {
                REQUIRE(call_count == 1);
            }
        }

        WHEN("it processes a function returning unavailable")
        {            
            int call_count = 0;
            mse::Context ctx;
            auto start_time = std::chrono::system_clock::now();
            mse::Status status = retry_request_hook->Process([&](mse::Context&) {
                ++call_count;
                return mse::Status{mse::StatusCode::unavailable, ""};
            }, ctx);            

            THEN("status is unavailable")
            {
                REQUIRE(status.code == mse::StatusCode::unavailable);
            }
            AND_THEN("function has been called four times")
            {
                REQUIRE(call_count == 4);
            }
            AND_THEN("duration is about 30ms")
            {
                auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - start_time);
                REQUIRE(duration.count() == Catch::Approx((30ms).count()).epsilon(0.33)); // large epsilon because this one is not very reproducable
            }
        }
    }
}
