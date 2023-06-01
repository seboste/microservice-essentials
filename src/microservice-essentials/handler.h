#pragma once

#include <string>

namespace mse
{

/*
    Abstract base class for a handler.

    The Handle() method shall be called from the main function and block until Stop() is beeing called.
    The Stop() method is automatically called when the service is requested to shutdown. @see
   cross-cutting-concerns/graceful-shutdown.h

    Note that the handler name must be unique for each instance. Otherwise an exception will be thrown during
   construction.
 */
class Handler
{
public:
  Handler(const std::string& name = "handler");
  ~Handler();

  const std::string& GetName() const
  {
    return _name;
  }

  virtual void Handle() = 0;
  virtual void Stop() = 0;

protected:
private:
  const std::string _name;
};

} // namespace mse
