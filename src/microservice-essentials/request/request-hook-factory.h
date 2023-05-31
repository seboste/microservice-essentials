#pragma once

#include <any>
#include <functional>
#include <memory>
#include <microservice-essentials/request/request-hook.h>
#include <typeindex>
#include <typeinfo>
#include <unordered_map>

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

  template <typename ParameterType> void Register(FactoryMethod factory_method)
  {
    Register(typeid(ParameterType), factory_method);
  }
  void Register(const std::type_info& hook_construction_parameter_type, FactoryMethod factory_method);

  std::unique_ptr<RequestHook> Create(const std::any& hook_construction_parameters);
  void Clear();

private:
  RequestHookFactory();

  std::unordered_map<std::type_index, FactoryMethod> _factory_methods;
};

/**
 * During first creation of an instance of this class, a Create method is registered to the
 * RequestHookFactory that generates a RequestHookT based on ParametersT.
 * Best practise is to have an instance of this class as a member of a Parameter struct so
 * that the corresponding RequestHook can be created based on that Parameter struct.
 */
template <typename ParametersT, typename RequestHookT> class AutoRequestHookParameterRegistration
{
public:
  static std::unique_ptr<RequestHookT> Create(const std::any& parameters)
  {
    return std::make_unique<RequestHookT>(std::any_cast<ParametersT>(parameters));
  }

  AutoRequestHookParameterRegistration()
  {
    static bool registration_done = []() {
      // this static variable along with the inialization makes sure that the registration
      // is done exactly once when the first AutoRequestHookParameterRegistration is created.
      mse::RequestHookFactory::GetInstance().Register<ParametersT>(Create);
      return true;
    }();
    (void)registration_done;
  }
};

} // namespace mse