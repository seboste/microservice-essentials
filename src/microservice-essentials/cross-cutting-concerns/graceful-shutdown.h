#pragma once

#include <atomic>
#include <functional>
#include <microservice-essentials/utilities/signal-handler.h>
#include <unordered_map>

namespace mse
{

/**
    Singleton that calls registered named callbacks if a shutdown is requested.

    Beware that requesting a shutdown must not occur before registering callbacks has been completed because this class
   is not thread-safe.
 */
class GracefulShutdown
{
public:
  static GracefulShutdown& GetInstance();

  void Register(const std::string& id, std::function<void(void)> shutdown_callback);
  void UnRegister(const std::string& id);

  void RequestShutdown();
  bool IsShutdownRequested() const;

protected:
  GracefulShutdown();
  virtual ~GracefulShutdown();

private:
  std::unordered_map<std::string, std::function<void(void)>> _callbacks;
  std::atomic<bool> _isShutdownRequested =
      false; // use atomic because RequestShutdown() and IsShutdownRequested() may be called from different threads
};

/**
    Connects a system signal (typically SIGTERM) to the GracefulShutdown Singleton so that the service shutdown is
   requested upon receiving that signal

    Should be instantiated in the service's main function AFTER all callbacks have been registered to the
   GracefulShutdown
 */
class GracefulShutdownOnSignal
{
public:
  GracefulShutdownOnSignal(Signal signal = Signal::SIG_SHUTDOWN)
      : _signalHandler(signal, []() { GracefulShutdown::GetInstance().RequestShutdown(); })
  {
  }
  virtual ~GracefulShutdownOnSignal()
  {
  }

private:
  SignalHandler _signalHandler;
};

} // namespace mse
