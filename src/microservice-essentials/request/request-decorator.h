#pragma once

#include <microservice-essentials/status.h>
#include <microservice-essentials/context.h>
#include <functional>

namespace mse
{

class RequestDecorator
{
public:
    typedef std::function<Status(Context& context)> Func;   

    RequestDecorator(const std::string& name, Context& context);
    virtual ~RequestDecorator();
    
    virtual Status Process(Func func);
protected:
    virtual Status pre_process();
    virtual Status post_process();

    Context& _context;
    std::string _name;
};

}