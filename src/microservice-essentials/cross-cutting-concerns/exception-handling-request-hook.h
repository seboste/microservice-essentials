#pragma once

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

    typedef std::pair<std::shared_ptr<ExceptionHandling::ExceptionTypePredicate>, mse::Status> ExceptionToStatusMapping;
    
    struct Parameters
    {
        Parameters(const std::vector<ExceptionToStatusMapping>& mappings = ExceptionHandlingRequestHook::_default_exception_to_status_mappings);
        std::vector<ExceptionToStatusMapping> exception_to_status_mapping  = ExceptionHandlingRequestHook::_default_exception_to_status_mappings;
        AutoRequestHookParameterRegistration<ExceptionHandlingRequestHook::Parameters, ExceptionHandlingRequestHook> auto_registration;
    };

    ExceptionHandlingRequestHook(const Parameters& parameters);
    virtual ~ExceptionHandlingRequestHook();    

    virtual Status Process(Func func, Context& context) override;

private:
    std::vector<ExceptionToStatusMapping> _exception_to_status_mappings;    
    static const std::vector<ExceptionToStatusMapping> _default_exception_to_status_mappings;
};

}

#include "exception-handling-request-hook.txx"

