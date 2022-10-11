#pragma once

#include <microservice-essentials/request/request-hook.h>
#include <microservice-essentials/context.h>
#include <any>
#include <memory>
#include <vector>

namespace mse
{

class RequestProcessor
{
public:
    RequestProcessor(const std::string& request_name, mse::Context&& context);

    RequestProcessor& With(std::unique_ptr<RequestHook>&& hook);
    RequestProcessor& With(const std::any& hook_construction_params);
    static void GloballyWith(const std::any& hook_construction_params);

    Status Process(RequestHook::Func func);
    
    static void ClearGlobalHooks();
private:
    std::vector<std::unique_ptr<RequestHook>> _hooks;
    std::string _request_name;
    mse::Context _context;

    static std::vector<std::any> _global_hook_construction_params;
};

}