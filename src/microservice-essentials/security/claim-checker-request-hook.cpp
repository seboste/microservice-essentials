#include "claim-checker-request-hook.h"
#include <sstream>

using namespace mse;

ClaimCheckerRequestHook::Parameters::Parameters(const std::string& claim_, Predicate checker_, Status fail_status_)
    : claim(claim_)
    , checker(checker_)
    , fail_status(fail_status_)    
{
}

Status ClaimCheckerRequestHook::PermissionDenied{StatusCode::permission_denied, "insufficient claims"};
    
ClaimCheckerRequestHook::Parameters ClaimCheckerRequestHook::ClaimEqualTo(const std::string& claim, const std::string& value, mse::Status fail_status)
{    
    return Parameters(claim, [value](const std::string& c)->bool { return c == value;}, fail_status);
}

ClaimCheckerRequestHook::Parameters ClaimCheckerRequestHook::ClaimInSpaceSeparatedList(const std::string& claim, const std::string& value, mse::Status fail_status)
{    
    return Parameters(claim, 
        [value](const std::string& c)->bool
        { 
            std::istringstream iss(c);
            std::string token;
            while(iss >> token)
            {                
                if(value == token)
                {
                    return true;
                }
            }
            return false;
        },
        fail_status);
}

ClaimCheckerRequestHook::ClaimCheckerRequestHook(const Parameters& parameters)
    : mse::RequestHook("claim checker")
    , _params(parameters)
{
}

Status ClaimCheckerRequestHook::pre_process(Context& context)
{
    if(!context.Contains(_params.claim))
    {        
        return _params.fail_status;
    }
    
    const std::string claim = context.At(_params.claim);    
    if(!_params.checker(claim))
    {
        return _params.fail_status;
    }

    return Status::OK;
}
