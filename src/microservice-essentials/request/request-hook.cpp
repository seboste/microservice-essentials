#include "request-hook.h"
#include <microservice-essentials/observability/logger.h>

using namespace mse;

RequestHook::RequestHook(const std::string& name) : _name(name)
{
}

RequestHook::~RequestHook()
{
}

Status RequestHook::Process(Func func, Context& context)
{
  MSE_LOG_TRACE(std::string("preprocessing by ") + _name);

  if (Status s = pre_process(context); !s)
  {
    MSE_LOG_TRACE(std::string("preprocessing failed with status ") + to_string(s.code) + " (" + s.details + ")");
    return s;
  }

  MSE_LOG_TRACE(std::string("processing by ") + _name);

  Status status = Status::OK;
  try
  {
    status = func(context);
  }
  catch (const std::exception& e)
  {
    MSE_LOG_TRACE(std::string("postprocessing by ") + _name + " during exception: " + e.what());
    status = post_process(context, mse::Status{mse::StatusCode::invalid, "exception during request"});
    if (status.code == mse::StatusCode::invalid)
    {
      throw;
    }
    MSE_LOG_TRACE(std::string("completely processed by ") + _name + "after recovering from exception");
    return status;
  }
  catch (...)
  {
    MSE_LOG_TRACE(std::string("postprocessing by ") + _name + " during unknown exception");
    status = post_process(context, mse::Status{mse::StatusCode::invalid, "exception during request"});
    if (status.code == mse::StatusCode::invalid)
    {
      throw;
    }
    MSE_LOG_TRACE(std::string("completely processed by ") + _name + "after recovering from exception");
    return status;
  }

  if (!status)
  {
    MSE_LOG_TRACE(std::string("processing failed with status ") + to_string(status.code) + " (" + status.details + ")");
  }

  MSE_LOG_TRACE(std::string("postprocessing by ") + _name);
  status = post_process(context, status);
  if (!status)
  {
    MSE_LOG_TRACE(std::string("postprocessing failed with status ") + to_string(status.code) + " (" + status.details +
                  ")");
  }

  MSE_LOG_TRACE(std::string("completely processed by ") + _name);
  return status;
}

Status RequestHook::pre_process(Context&)
{
  return Status::OK;
}

Status RequestHook::post_process(Context&, Status status)
{
  return status;
}