#pragma once

#include <microservice-essentials/request/request-hook.h>
#include <microservice-essentials/request/request-type.h>
#include <microservice-essentials/context.h>
#include <any>
#include <memory>
#include <vector>

namespace mse
{

/**
 * Base class that wraps a request to add technical/non-business related patterns to that request (e.g. reliability, security, ...)
 * by calling hooks before and after handling a request.
 * Globally registered hooks are always included in the request handling.
 * do not use this class directly, instead use RequestHandler for incoming and RequestIssuer for outgoing requests.
 * 
 * Example: Assume you have two hooks A and B with corresponding A::Params and B::Params parameter classes from with the hooks can be constructed:
 * 
 * //somewhere in main
 * RequestHandler::GloballyWith(A::Params({}));     //makes sure that hook a is used for all incoming requests
 * 
 * //in the handler method for the specific call "myRequest"
 * RequestHandler("myRequest", mse::Context())
 *          .With(B::Params({}))
 *          .Process([&](mse::Context& context)
 *          {                
 *              _api.MyRequest();
 *              return Status::OK;
 *          }
 *     );        
 * 
 *  In the Process-Method, the following functions will called in the following order:
 *  1. A::pre_process()
 *  2. B::pre_process()
 *  3. The lambda
 *  4. B::post_process()
 *  5. A::post_process()
 * 
 *  If pre_process fails, the following functions are typically not executed.
 *  Hooks are allowed to change the context object.
 * 
*/
class RequestProcessor
{
public:
    RequestProcessor(const std::string& request_name, mse::RequestType request_type, mse::Context&& context);

    RequestProcessor& With(std::unique_ptr<RequestHook>&& hook);
    RequestProcessor& With(const std::any& hook_construction_params);    

    virtual Status Process(RequestHook::Func func);

protected:
    std::vector<std::unique_ptr<RequestHook>> _hooks;
    std::string _request_name;
    mse::RequestType _request_type;
    mse::Context _context;    
};

/**
 * Template class that allows to define hooks that shall be called for each and every request the service receives or issues (globally).
 */
template<typename RequestProcessorType> 
class GlobalRequestHookConstructionHolder
{
    public:
        GlobalRequestHookConstructionHolder(RequestProcessorType& requestProcessor);

        static void GloballyWith(const std::any& hook_construction_params) { _global_hook_construction_params.push_back(hook_construction_params); }
        static void ClearGlobalHooks() { _global_hook_construction_params.clear(); }
    private:
        static std::vector<std::any> _global_hook_construction_params;
};

/** 
 * RequestProcessor for incoming request (i.e. this service handles those requests)
 * Allows to define hooks that shall be called for each incoming request.
 * Before calling the base class, the thread local context is set so that all code that is executed during request handling 
 * is able to access the context without the need to pass it around explicitly.
*/
class RequestHandler : public RequestProcessor, public GlobalRequestHookConstructionHolder<RequestHandler>
{
    public:
        RequestHandler(const std::string& request_name, mse::Context&& context);

        virtual Status Process(RequestHook::Func func) override;
};

/**
 * RequestProcessor for outgoing request (i.e. this service issues those requests to other services)
 * Allows to define hooks that shall be called for each outgoing request.
*/
class RequestIssuer : public RequestProcessor, public GlobalRequestHookConstructionHolder<RequestIssuer>
{
    public:
        RequestIssuer(const std::string& request_name, mse::Context&& context);
};

//explicit instantiation declaration to suppress warning
extern template std::vector<std::any> GlobalRequestHookConstructionHolder<RequestHandler>::_global_hook_construction_params;
extern template std::vector<std::any> GlobalRequestHookConstructionHolder<RequestIssuer>::_global_hook_construction_params;

}