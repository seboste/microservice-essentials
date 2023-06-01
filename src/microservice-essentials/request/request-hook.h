#pragma once

#include <functional>
#include <microservice-essentials/context.h>
#include <microservice-essentials/request/request-type.h>
#include <microservice-essentials/status.h>

namespace mse
{

/**
 * Base class for all request hooks. Derived classes can be used to add business independent behavior to a request (e.g.
 * reliability, security, ...) See request-processor.h for more details. The default behavior of the Process method is
 * to call pre_process before and post_process after the passed function is called. The funcion and post_process will
 * only be called if pre_process succeeds (status code ok). The default implementation of pre_process and post_process
 * is empty.
 */
class RequestHook
{
public:
  typedef std::function<Status(Context& context)> Func;

  RequestHook(const std::string& name);
  virtual ~RequestHook();

  RequestType GetRequestType() const
  {
    return _type;
  }
  void SetRequestType(RequestType request_type)
  {
    _type = request_type;
  }

  virtual Status Process(Func func, Context& context);

protected:
  virtual Status pre_process(Context& context);
  virtual Status post_process(Context& context, Status status);

  const std::string _name;
  RequestType _type = RequestType::invalid;
};

} // namespace mse