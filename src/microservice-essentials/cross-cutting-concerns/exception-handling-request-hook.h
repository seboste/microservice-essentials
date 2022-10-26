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
    class ExceptionTypePredicate
    {
    public:
        virtual ~ExceptionTypePredicate() = default;
        virtual bool Test(const std::exception_ptr& exception) const = 0;
    };    

    template<typename ExceptionType>
    class ExceptionOfType : public ExceptionTypePredicate
    {
        public:
            bool Test(const std::exception_ptr& exception) const override;
    };

    class AnyException : public ExceptionTypePredicate
    {
        public:
            bool Test(const std::exception_ptr&) const override { return true;}
    };

    template<typename Predicate>
    std::shared_ptr<ExceptionTypePredicate> Is() { return std::make_shared<Predicate>(); }
} //namespace ExceptionHandling

class ExceptionHandlingRequestHook : public mse::RequestHook
{
public:

    struct ExceptionHandlingDefinition
    {
        std::shared_ptr<ExceptionHandling::ExceptionTypePredicate> exception_predicate;
        mse::Status status;
        mse::LogLevel log_level = mse::LogLevel::invalid; // do not log by default
        bool forward_exception_details_to_caller = false; // do not leak private details by default
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

