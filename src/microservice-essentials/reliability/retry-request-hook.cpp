#include "retry-request-hook.h"
#include <microservice-essentials/observability/logger.h>
#include <cmath>
#include <random>
#include <thread>

using namespace mse;

namespace {

using Duration = RetryBackoffStrategy::Duration;

std::optional<Duration> getDurationUntilNextRetry(uint32_t retry_count, uint32_t max_retry_count, Duration total_request_duration, Duration max_total_request_duration)
{
    using namespace std::chrono_literals;

    //maximum number of retries reached?
    if(retry_count > max_retry_count)
    {
        //don't try again
        return std::nullopt;
    }

    //timeout reached?
    if(total_request_duration > max_total_request_duration)
    {
        //timepoint has already passed => retry immediately
        return 0ms;
    }

    //compute the duration until next retry
    return max_total_request_duration - total_request_duration;
}

}

LinearRetryBackoff::LinearRetryBackoff(uint32_t max_retry_count, Duration retry_interval)
    : _max_retry_count(max_retry_count)
    , _retry_interval(retry_interval)
{
}

std::optional<RetryBackoffStrategy::Duration> LinearRetryBackoff::GetDurationUntilNextRetry(uint32_t retry_count, Duration total_request_duration) const
{
    return getDurationUntilNextRetry(retry_count, _max_retry_count, total_request_duration, 
        _retry_interval * retry_count   //linear
    );
}

ExponentialRetryBackoff::ExponentialRetryBackoff(uint32_t max_retry_count, Duration first_retry_interval, float base)
    : _max_retry_count(max_retry_count)
    , _first_retry_interval(first_retry_interval)
    , _base(base)
{
}

std::optional<RetryBackoffStrategy::Duration> ExponentialRetryBackoff::GetDurationUntilNextRetry(uint32_t retry_count, Duration total_request_duration) const
{
    //sum up exponentially growing intervals
    using namespace std::chrono_literals;
    Duration max_total_request_duration = 0ms;
    for(uint32_t i=0; i<retry_count; ++i)
    {
        max_total_request_duration += _first_retry_interval * std::pow(_base, static_cast<float>(i));
    }

    return getDurationUntilNextRetry(retry_count, _max_retry_count, total_request_duration, max_total_request_duration);
}

BackoffGaussianJitterDecorator::BackoffGaussianJitterDecorator(std::shared_ptr<RetryBackoffStrategy> strategy, Duration sigma)    
    : _backoff_strategy(strategy)
    , _sigma(sigma)
{
}

std::optional<Duration> BackoffGaussianJitterDecorator::GetDurationUntilNextRetry(uint32_t retry_count, Duration total_request_duration) const
{
    std::optional<Duration> duration = _backoff_strategy->GetDurationUntilNextRetry(retry_count, total_request_duration);
    if(!duration.has_value())
    {
        return std::nullopt;
    }

    static std::default_random_engine generator;
    std::normal_distribution<double> distribution(duration.value().count(), _sigma.count());
    return Duration(distribution(generator));
}

RetryRequestHook::Parameters::Parameters(std::shared_ptr<RetryBackoffStrategy> strategy)
    : backoff_strategy(strategy)
{   
}

RetryRequestHook::RetryRequestHook(const Parameters& parameters)
    : RequestHook("retry")
    , _parameters(parameters)
{
}

Status RetryRequestHook::Process(Func func, Context& context)
{
    _request_start_time = Clock::now();
    _retry_counter = 0;
    _func = func;
    return RequestHook::Process(func, context);
}
    
Status RetryRequestHook::post_process(Context& context, Status status)
{
    if(status)
    {
        //success => no retry
        return status;
    }

    std::optional<RetryBackoffStrategy::Duration> duration_until_next_retry = 
        _parameters.backoff_strategy->GetDurationUntilNextRetry(++_retry_counter, Clock::now() - _request_start_time);
    if(!duration_until_next_retry.has_value())
    {
        MSE_LOG_TRACE(std::string("Retrying request failed with ") + to_string(status.code) + " after " + std::to_string(_retry_counter) 
                + " retries. No further retry requested.");
        return status;
    }

    MSE_LOG_TRACE(std::string("Request returned ") + to_string(status.code) + ". Retry #" + std::to_string(_retry_counter) 
                + " in " + std::to_string(duration_until_next_retry.value().count()) + " ms.");

    std::this_thread::sleep_for(duration_until_next_retry.value());
    return RequestHook::Process(_func, context);
}

