#pragma once

#include <microservice-essentials/request/request-hook.h>
#include <microservice-essentials/request/request-hook-factory.h>
#include <chrono>
#include <memory>
#include <optional>
#include <set>

namespace mse
{

//abstract backoff strategy for RetryRequestHook
class RetryBackoffStrategy
{
public:
    virtual ~RetryBackoffStrategy() = default;

    using Duration = std::chrono::duration<double, std::milli>;

    //returns the duration until the next retry shall be attempted based on the current retry_count (will be called with 
    //a value of 1 for the first time) and the duration the request has already taken. Should return 0ms if a retry shall be
    //performed instantly.
    //In case no further attemps shall be taken, nullopt is returned.
    virtual std::optional<Duration> GetDurationUntilNextRetry(uint32_t retry_count, Duration total_request_duration) const = 0;
};

//retries in regular intervals (e.g. after 10, 20, 30 ms for _max_retry_count==3 and _retry_interval==10ms)
class LinearRetryBackoff : public RetryBackoffStrategy
{
public:
    LinearRetryBackoff(uint32_t max_retry_count, Duration retry_interval);
    virtual std::optional<Duration> GetDurationUntilNextRetry(uint32_t retry_count, Duration total_request_duration) const override;
private:
    uint32_t _max_retry_count;
    Duration _retry_interval;
};

//retries in exponential increasing intervals (e.g. after 10, 10+20, 10+20+40 ms for _max_retry_count==3, _first_retry_interval==10ms, and _base==2)
class ExponentialRetryBackoff : public RetryBackoffStrategy
{
public:
    ExponentialRetryBackoff(uint32_t max_retry_count, Duration first_retry_interval, float base = 2.0f);
    virtual std::optional<Duration> GetDurationUntilNextRetry(uint32_t retry_count, Duration total_request_duration) const override;

private:
    uint32_t _max_retry_count;
    Duration _first_retry_interval;
    float _base;
};

//prevents retry storm by adding gaussian distributed random jitter to a strategy's duration until next retry
class BackoffGaussianJitterDecorator : public RetryBackoffStrategy
{
    public:
        BackoffGaussianJitterDecorator(std::shared_ptr<RetryBackoffStrategy> strategy, Duration sigma);
        virtual std::optional<Duration> GetDurationUntilNextRetry(uint32_t retry_count, Duration total_request_duration) const override;
    private:
        std::shared_ptr<RetryBackoffStrategy> _backoff_strategy;
        Duration _sigma;
};

/**
 * Request hook for outgoing requests that retries requests that return certain error codes
 * after durations defined by a backoff strategy (e.g. ExponentialRetryBackoff)
 * 
 * To avoid a retry storm (i.e. many clients retrying at the very same time), it is recommended to randomly
 * apply jitter to the retry intervalls (see BackoffGaussianJitterDecorator).
 * 
 * In case retries shall be performed in case of exceptions, consider using the ExceptionHandlingRequestHook to
 * convert an exception to a status code that results in retries.
 * 
*/
class RetryRequestHook : public mse::RequestHook
{
public:
    using Clock = std::chrono::system_clock;
    using TimePoint = std::chrono::time_point<Clock>;

    struct Parameters
    {
        Parameters(
            std::shared_ptr<RetryBackoffStrategy> strategy,
            const std::set<mse::StatusCode>& retry_ec = { mse::StatusCode::unavailable, mse::StatusCode::resource_exhausted, mse::StatusCode::internal, mse::StatusCode::unknown }
            );
        
        std::shared_ptr<RetryBackoffStrategy> backoff_strategy;
        std::set<mse::StatusCode> retry_error_codes;
        AutoRequestHookParameterRegistration<RetryRequestHook::Parameters, RetryRequestHook> auto_registration;
    };

    RetryRequestHook(const Parameters& parameters);
    virtual ~RetryRequestHook() = default;
    
    virtual Status Process(Func func, Context& context) override;

protected:    
    virtual Status post_process(Context& context, Status status) override;
private:
    Parameters _parameters;
    TimePoint _request_start_time;
    uint32_t _retry_counter = 0;
    Func _func;
};

}
