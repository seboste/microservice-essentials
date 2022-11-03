#include "retry-request-hook.h"
#include <thread>
#include <microservice-essentials/observability/logger.h>

using namespace mse;

LinearRetryBackoff::LinearRetryBackoff(uint32_t max_retry_count, Duration retry_interval)
    : _max_retry_count(max_retry_count)
    , _retry_interval(retry_interval)
{
}

std::optional<RetryBackoffStrategy::Duration> LinearRetryBackoff::GetDurationUntilNextRetry(uint32_t retry_count, Duration total_request_duration) const
{
    using namespace std::chrono_literals;

    //maximum number of retries reached?
    if(retry_count > _max_retry_count)
    {
        //don't try again
        return std::nullopt;
    }

    //timeout reached?
    Duration retry_duration = _retry_interval * retry_count;
    if(total_request_duration > retry_duration)
    {
        //don't try again
        return std::nullopt;
    }


    //compute the duration until next retry
    return retry_duration - total_request_duration;
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

