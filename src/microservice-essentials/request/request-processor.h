#pragma once

#include <microservice-essentials/request/request-hook.h>
#include <microservice-essentials/context.h>
#include <memory>
#include <vector>

namespace mse
{

class RequestProcessor
{
public:
    RequestProcessor(const std::string& request_name, mse::Context&& context);

    RequestProcessor& With(std::unique_ptr<RequestHook>&& hook);

    Status Process(RequestHook::Func func);
private:
    std::vector<std::unique_ptr<RequestHook>> _hooks;
    std::string _request_name;
    mse::Context _context;
};

}