#include "request-decorator.h"
#include <microservice-essentials/observability/logger.h>

using namespace mse;


RequestDecorator::RequestDecorator(const std::string& name, Context& context)
    : _context(context)
    , _name(name)
{
}

RequestDecorator::~RequestDecorator()
{
}
    
Status RequestDecorator::Process(Func func)
{
    MSE_LOG_TRACE(std::string("preprocessing by ") + _name);

    if(Status s = pre_process(); !s)
    {
        MSE_LOG_TRACE(std::string("preprocessing failed with status ") + to_string(s.code) + " (" + s.details + ")" );
        return s;
    }

    MSE_LOG_TRACE(std::string("processing by ") + _name);
    
    if(Status s = func(_context); !s)
    {
        MSE_LOG_TRACE(std::string("processing failed with status ") + to_string(s.code) + " (" + s.details + ")" );
        return s;
    }

    MSE_LOG_TRACE(std::string("postprocessing by ") + _name);

    if(Status s = post_process(); !s)
    {
        MSE_LOG_TRACE(std::string("postprocessing failed with status ") + to_string(s.code) + " (" + s.details + ")" );
        return s;
    }

    MSE_LOG_TRACE(std::string("completely processed by ") + _name);
    return { StatusCode::ok };
}
