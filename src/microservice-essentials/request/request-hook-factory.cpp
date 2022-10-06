#include "request-hook-factory.h"
#include <iostream>

using namespace mse;

RequestHookFactory& RequestHookFactory::GetInstance()
{
    static RequestHookFactory instance;
    return instance;
}   

void RequestHookFactory::Register(const std::type_info& hook_construction_parameter_type, FactoryMethod factory_method)
{
    std::cout << "registering " << hook_construction_parameter_type.name() << std::endl;
    if(_factory_methods.find(std::type_index(hook_construction_parameter_type)) != _factory_methods.end())
    {
        //key already exists
        throw std::invalid_argument("typeid already exists");
    }

    _factory_methods[std::type_index(hook_construction_parameter_type)] = factory_method;
}

std::unique_ptr<RequestHook> RequestHookFactory::Create(const std::any& hook_construction_parameters)
{
    std::cout << "creating " << hook_construction_parameters.type().name() << std::endl;

    const auto cit = _factory_methods.find(std::type_index(hook_construction_parameters.type()));
    if(cit == _factory_methods.end())
    {
        throw std::out_of_range("cannot create request hook. Unknown typeid.");
    }
    return cit->second(hook_construction_parameters);
}

void RequestHookFactory::Clear()
{
    _factory_methods.clear();
}

RequestHookFactory::RequestHookFactory()
{
}
