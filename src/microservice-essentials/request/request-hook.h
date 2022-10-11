#pragma once

#include <microservice-essentials/status.h>
#include <microservice-essentials/context.h>
#include <microservice-essentials/request/request-type.h>
#include <functional>

namespace mse
{

class RequestHook
{
public:
    typedef std::function<Status(Context& context)> Func;

    RequestHook(const std::string& name);
    virtual ~RequestHook();

    RequestType GetRequestType() const { return _type; }
    void SetRequestType(RequestType request_type) { _type = request_type; }

    virtual Status Process(Func func, Context& context);    

protected:
    virtual Status pre_process(Context& context);
    virtual Status post_process(Context& context, Status status);
    
    const std::string _name;
    RequestType _type = RequestType::invalid;
};

}