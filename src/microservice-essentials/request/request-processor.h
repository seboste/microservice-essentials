#pragma once

#include <microservice-essentials/request/request-decorator.h>
#include <microservice-essentials/context.h>
#include <memory>
#include <vector>

namespace mse
{

class RequestProcessor
{
public:
    RequestProcessor(const std::string& request_name, mse::Context&& context);

    RequestProcessor& With(std::unique_ptr<RequestDecorator>&& decorator);

    Status Process(RequestDecorator::Func func);
private:
    std::vector<std::unique_ptr<RequestDecorator>> _decorators;
    std::string _request_name;
    mse::Context _context;
};

}