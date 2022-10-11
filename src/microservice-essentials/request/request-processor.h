#pragma once

#include <microservice-essentials/request/request-hook.h>
#include <microservice-essentials/request/request-type.h>
#include <microservice-essentials/context.h>
#include <any>
#include <memory>
#include <vector>

namespace mse
{

class RequestProcessor
{
public:
    RequestProcessor(const std::string& request_name, mse::RequestType request_type, mse::Context&& context);

    RequestProcessor& With(std::unique_ptr<RequestHook>&& hook);
    RequestProcessor& With(const std::any& hook_construction_params);    

    virtual Status Process(RequestHook::Func func);

protected:
    std::vector<std::unique_ptr<RequestHook>> _hooks;
    std::string _request_name;
    mse::RequestType _request_type;
    mse::Context _context;    
};

template<typename RequestProcessorType> 
class GlobalRequestHookConstructionHolder
{
    public:
        GlobalRequestHookConstructionHolder(RequestProcessorType& requestProcessor);

        static void GloballyWith(const std::any& hook_construction_params) { _global_hook_construction_params.push_back(hook_construction_params); }
        static void ClearGlobalHooks() { _global_hook_construction_params.clear(); }
    private:
        static std::vector<std::any> _global_hook_construction_params;
};

class RequestHandler : public RequestProcessor, public GlobalRequestHookConstructionHolder<RequestHandler>
{
    public:
        RequestHandler(const std::string& request_name, mse::Context&& context);

        virtual Status Process(RequestHook::Func func) override;
};

class RequestIssuer : public RequestProcessor, public GlobalRequestHookConstructionHolder<RequestIssuer>
{
    public:
        RequestIssuer(const std::string& request_name, mse::Context&& context);
};


    


}