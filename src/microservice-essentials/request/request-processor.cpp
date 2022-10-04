#include "request-processor.h"
#include <microservice-essentials/context.h>

using namespace mse;

RequestProcessor::RequestProcessor(const std::string& request_name, mse::Context&& context)
    : _request_name(request_name)
    , _context(std::move(context))
{
}

RequestProcessor& RequestProcessor::With(std::unique_ptr<RequestDecorator>&& decorator)
{
    _decorators.emplace_back(std::move(decorator));
    return *this;
}

Status RequestProcessor::Process(RequestDecorator::Func func)
{
    //1. make given context available as the thread local context
    Context::GetThreadLocalContext() = _context;
    
    //2. create nested wrapper
    RequestDecorator::Func wrapper = func;
    for(auto decorator_cit = rbegin(_decorators); decorator_cit != rend(_decorators); ++decorator_cit)
    {
        RequestDecorator& decorator = **decorator_cit;
        wrapper = [&decorator, wrapper](mse::Context& context)->mse::Status 
        { 
            return decorator.Process(wrapper, context);
        };
    }

    //3. execute all decorators and the func with a new context    
    mse::Context request_context({{"request", _request_name}}, &_context);
    return wrapper(request_context);
}
