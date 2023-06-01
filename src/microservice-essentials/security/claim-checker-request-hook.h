#pragma once

#include <functional>
#include <microservice-essentials/request/request-hook-factory.h>
#include <microservice-essentials/request/request-hook.h>
#include <microservice-essentials/status.h>

namespace mse
{

/**
 * Request Hook for checking claims by a predicate (Authorization).
 * A typical use case is to deny access if the caller does not have enough privilege to access a certain endpoint.
 * This can be achieved by adding a TokenAuthRequestHook globally, which extracts a claim (e.g. the scope) from a token
 * and adds it to the current context. ClaimCheckerRequestHooks with different predicates (e.g. one that checks if the
 * required scope is within the list of scopes in the context) can then be added to each handler individually.
 */
class ClaimCheckerRequestHook : public mse::RequestHook
{
public:
  struct Parameters
  {
    typedef std::function<bool(const std::string&)> Predicate;
    Parameters(const std::string& claim_, Predicate checker_, mse::Status fail_status_);

    std::string claim;
    Predicate checker;
    mse::Status fail_status;

    AutoRequestHookParameterRegistration<ClaimCheckerRequestHook::Parameters, ClaimCheckerRequestHook>
        auto_registration;
  };

  static mse::Status PermissionDenied;
  static Parameters ClaimEqualTo(const std::string& claim, const std::string& value,
                                 mse::Status fail_status = PermissionDenied);
  static Parameters ClaimInSpaceSeparatedList(const std::string& claim, const std::string& value,
                                              mse::Status fail_status = PermissionDenied);
  static Parameters ScopeContains(const std::string& value)
  {
    return ClaimInSpaceSeparatedList("scope", value);
  }

  ClaimCheckerRequestHook(const Parameters& parameters);
  virtual ~ClaimCheckerRequestHook() = default;

protected:
  virtual Status pre_process(Context& context) override;

private:
  Parameters _params;
};

} // namespace mse
