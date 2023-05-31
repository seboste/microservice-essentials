#include "request-processor.h"
#include <microservice-essentials/context.h>
#include <microservice-essentials/request/request-hook-factory.h>

using namespace mse;

RequestProcessor::RequestProcessor(const std::string& request_name, mse::RequestType request_type,
                                   mse::Context&& context)
    : _request_name(request_name), _request_type(request_type), _context(std::move(context))
{
}

RequestProcessor& RequestProcessor::With(std::unique_ptr<RequestHook>&& hook)
{
  hook->SetRequestType(_request_type);
  _hooks.emplace_back(std::move(hook));
  return *this;
}

RequestProcessor& RequestProcessor::With(const std::any& hook_construction_params)
{
  return With(RequestHookFactory::GetInstance().Create(hook_construction_params));
}

Status RequestProcessor::Process(RequestHook::Func func)
{
  // 1. create nested wrapper
  RequestHook::Func wrapper = func;
  for (auto hook_cit = rbegin(_hooks); hook_cit != rend(_hooks); ++hook_cit)
  {
    RequestHook& hook = **hook_cit;
    wrapper = [&hook, wrapper](mse::Context& context) -> mse::Status { return hook.Process(wrapper, context); };
  }

  // 2. execute all hooks and the func with a new context
  mse::Context request_context({{"request", _request_name}}, &_context);
  return wrapper(request_context);
}

RequestHandler::RequestHandler(const std::string& request_name, mse::Context&& context)
    : RequestProcessor(request_name, RequestType::incoming, std::move(context)),
      GlobalRequestHookConstructionHolder(*this)
{
}

Status RequestHandler::Process(RequestHook::Func func)
{
  // make given context available as the thread local context
  Context::GetThreadLocalContext() = _context;

  return RequestProcessor::Process(func);
}

RequestIssuer::RequestIssuer(const std::string& request_name, mse::Context&& context)
    : RequestProcessor(request_name, RequestType::outgoing, std::move(context)),
      GlobalRequestHookConstructionHolder(*this)
{
}

template <typename RequestProcessorType>
GlobalRequestHookConstructionHolder<RequestProcessorType>::GlobalRequestHookConstructionHolder(
    RequestProcessorType& requestProcessor)
{
  for (const auto& params : _global_hook_construction_params)
  {
    requestProcessor.With(params);
  }
}

template <>
std::vector<std::any> GlobalRequestHookConstructionHolder<RequestHandler>::_global_hook_construction_params = {};
template <>
std::vector<std::any> GlobalRequestHookConstructionHolder<RequestIssuer>::_global_hook_construction_params = {};

// explicit instantiation
template class mse::GlobalRequestHookConstructionHolder<RequestHandler>;
template class mse::GlobalRequestHookConstructionHolder<RequestIssuer>;
