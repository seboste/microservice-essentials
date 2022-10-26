#pragma once

#include <microservice-essentials/observability/logger.h>
#include <microservice-essentials/request/request-hook.h>
#include <microservice-essentials/request/request-hook-factory.h>
#include <microservice-essentials/status.h>
#include <exception>
#include <vector>

namespace mse
{

namespace ExceptionHandling
{
    /**
     * Base class for predicates on exceptions.
    */
    class ExceptionTypePredicate
    {
    public:
        virtual ~ExceptionTypePredicate() = default;
        virtual bool Test(const std::exception_ptr& exception) const = 0;
    };

    /**
     * Exception type predicate that tests if an exception is of a specific type
     * or a subclass of that type.
    */
    template<typename ExceptionType>
    class ExceptionOfType : public ExceptionTypePredicate
    {
        public:
            bool Test(const std::exception_ptr& exception) const override;
    };

    /**
     * Exception type predicate that is true for all exceptions.
     */
    class AnyException : public ExceptionTypePredicate
    {
        public:
            bool Test(const std::exception_ptr&) const override { return true;}
    };

    //Helper to facilitate creation of a Predicate. Creation may look like: Is<ExceptionOfType<std::out_of_range>>()
    template<typename Predicate> std::shared_ptr<ExceptionTypePredicate> Is() { return std::make_shared<Predicate>(); }
} //namespace ExceptionHandling


/**
 * Request Hook for incoming requests that handles exceptions according to a exception handling definition:
 * - An exception can results in returning a predefined request status.
 * - Optionally the exception details can be forwarded in the status details (best practice for client side errors).
 * - Optionally a log entry with the exception details can be emitted (best practice for internal errors).
 * Note that exception details are only available for exceptions derived from std::exception.
 * The order of exception handling definitions defines the handling priority.
 * Any exception not matched by any of the exception handling definitions will be rethrown.
*/
class ExceptionHandlingRequestHook : public mse::RequestHook
{
public:    
    struct ExceptionHandlingDefinition
    {
        std::shared_ptr<ExceptionHandling::ExceptionTypePredicate> exception_predicate; // predicate that defines when this definition is to be applied
        mse::Status status;                                                             // status to be returned by the request
        mse::LogLevel log_level = mse::LogLevel::invalid;                               // do not log by default
        bool forward_exception_details_to_caller = false;                               // do not leak private details by default.
    };
        
    struct Parameters
    {
        Parameters(const std::vector<ExceptionHandlingDefinition>& exception_handling_definitions = ExceptionHandlingRequestHook::_default_exception_handling_definitions);
        std::vector<ExceptionHandlingDefinition> exception_handling_definitions  = ExceptionHandlingRequestHook::_default_exception_handling_definitions;
        AutoRequestHookParameterRegistration<ExceptionHandlingRequestHook::Parameters, ExceptionHandlingRequestHook> auto_registration;
    };

    ExceptionHandlingRequestHook(const Parameters& parameters);
    virtual ~ExceptionHandlingRequestHook();    

    virtual Status Process(Func func, Context& context) override;

private:
    Parameters _parameters;    
    static const std::vector<ExceptionHandlingDefinition> _default_exception_handling_definitions;
};

}

#include "exception-handling-request-hook.txx"

