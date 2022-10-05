#include "request-processor.h"
#include <microservice-essentials/context.h>

using namespace mse;

RequestProcessor::RequestProcessor(const std::string& request_name, mse::Context&& context)
    : _request_name(request_name)
    , _context(std::move(context))
{
}

RequestProcessor& RequestProcessor::With(std::unique_ptr<RequestHook>&& hook)
{
    _hooks.emplace_back(std::move(hook));
    return *this;
}

Status RequestProcessor::Process(RequestHook::Func func)
{
    //1. make given context available as the thread local context
    Context::GetThreadLocalContext() = _context;
    
    //2. create nested wrapper
    RequestHook::Func wrapper = func;
    for(auto hook_cit = rbegin(_hooks); hook_cit != rend(_hooks); ++hook_cit)
    {
        RequestHook& hook = **hook_cit;
        wrapper = [&hook, wrapper](mse::Context& context)->mse::Status 
        { 
            return hook.Process(wrapper, context);
        };
    }

    //3. execute all hooks and the func with a new context    
    mse::Context request_context({{"request", _request_name}}, &_context);
    return wrapper(request_context);
}
