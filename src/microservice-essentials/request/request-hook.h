#pragma once

#include <microservice-essentials/status.h>
#include <microservice-essentials/context.h>
#include <functional>

namespace mse
{

class RequestHook
{
public:
    typedef std::function<Status(Context& context)> Func;

    RequestHook(const std::string& name);
    virtual ~RequestHook();    

    virtual Status Process(Func func, Context& context);    

protected:
    virtual Status pre_process(Context& context);
    virtual Status post_process(Context& context);
    
    std::string _name;
};

}