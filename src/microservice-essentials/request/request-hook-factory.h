#pragma once

#include <microservice-essentials/request/request-hook.h>
#include <any>
#include <functional>
#include <unordered_map>
#include <memory>
#include <typeinfo>
#include <typeindex>

namespace mse
{

/**
 * Singleton factory for creating registered hooks based on a hook type specific parameter class
 * One or more parameter classes along with a custom factory method can be registered for each hook type.
 * The typeid of the parameter class passed as an std::any is used to call the correct factory method.
 * This factory is used by the request hook classes.
*/
class RequestHookFactory
{
public:
    static RequestHookFactory& GetInstance();
    typedef std::function<std::unique_ptr<RequestHook>(const std::any& hook_construction_parameters)> FactoryMethod;
    
    template<typename ParameterType>
    void Register(FactoryMethod factory_method) { Register(typeid(ParameterType), factory_method); }
    void Register(const std::type_info& hook_construction_parameter_type, FactoryMethod factory_method);
    
    std::unique_ptr<RequestHook> Create(const std::any& hook_construction_parameters);
    void Clear();

private:
    RequestHookFactory();

    std::unordered_map<std::type_index, FactoryMethod> _factory_methods;
};

}