#pragma once

#include <microservice-essentials/request/request-hook.h>
#include <any>
#include <functional>
#include <unordered_map>
#include <memory>
#include <typeindex>

namespace mse
{

class RequestHookFactory
{
public:
    static RequestHookFactory& GetInstance();
    typedef std::function<std::unique_ptr<RequestHook>(const std::any& hook_construction_parameters)> FactoryMethod;
    
    void Register(std::type_index hook_construction_parameter_type, FactoryMethod factory_method);
    std::unique_ptr<RequestHook> Create(const std::any& hook_construction_parameters);
    

private:
    RequestHookFactory();

    std::unordered_map<std::type_index, FactoryMethod> _factory_methods;
};

}