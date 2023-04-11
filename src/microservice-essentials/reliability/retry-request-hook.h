#pragma once

#include <microservice-essentials/request/request-hook.h>
#include <microservice-essentials/request/request-hook-factory.h>
#include <chrono>
#include <optional>
#include <memory>

namespace mse
{

class RetryBackoffStrategy
{
public:
    virtual ~RetryBackoffStrategy() = default;

    using Duration = std::chrono::duration<double, std::milli>;
    virtual std::optional<Duration> GetDurationUntilNextRetry(uint32_t retry_count, Duration total_request_duration) const = 0;
};

class LinearRetryBackoff : public RetryBackoffStrategy
{
public:
    LinearRetryBackoff(uint32_t max_retry_count, Duration retry_interval);
    virtual std::optional<Duration> GetDurationUntilNextRetry(uint32_t retry_count, Duration total_request_duration) const override;
private:
    uint32_t _max_retry_count;
    Duration _retry_interval;
};

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

class RetryRequestHook : public mse::RequestHook
{
public:
    using Clock = std::chrono::system_clock;
    using TimePoint = std::chrono::time_point<Clock>;

    struct Parameters
    {
        Parameters(std::shared_ptr<RetryBackoffStrategy> strategy);
        
        std::shared_ptr<RetryBackoffStrategy> backoff_strategy;
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
