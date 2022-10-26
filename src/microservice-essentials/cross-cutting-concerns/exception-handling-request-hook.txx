#include "exception-handling-request-hook.h"

template<typename ExceptionType>
bool mse::ExceptionHandling::ExceptionOfType<ExceptionType>::Test(const std::exception_ptr& exception) const
{
    try
    {
        std::rethrow_exception(exception);
    }
    catch(const ExceptionType&)
    {
        return true;
    }
    catch(...)
    {
        return false;
    }
}
