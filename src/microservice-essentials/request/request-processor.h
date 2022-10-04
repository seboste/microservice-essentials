#pragma once

#include <microservice-essentials/request/request-decorator.h>
#include <memory>
#include <vector>

namespace mse
{

class RequestProcessor
{
public:
    RequestProcessor& With(std::unique_ptr<RequestDecorator>&& decorator);

    Status Process(RequestDecorator::Func func);
private:
    std::vector<std::unique_ptr<RequestDecorator>> _decorators;
};

}