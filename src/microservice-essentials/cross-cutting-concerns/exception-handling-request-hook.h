#pragma once

#include <microservice-essentials/observability/logger.h>
#include <microservice-essentials/request/request-hook.h>
#include <microservice-essentials/request/request-hook-factory.h>
#include <microservice-essentials/status.h>
#include <exception>
#include <optional>
#include <vector>

namespace mse
{

namespace ExceptionHandling
{
    /**
     * Struct that defines how an exception shall be handled
    */
    struct Definition
    {      
        mse::Status status;                                  // status to be returned by the request
        mse::LogLevel log_level = mse::LogLevel::invalid;    // do not log by default
        bool forward_exception_details_to_caller = false;    // do not leak private details by default.
    };

    /**
     * Base class for mappers that map an exception to a handling definition
    */
    class Mapper
    {
    public:
        virtual ~Mapper() = default;

        //shall return nullopt_t in case the exception type is not handled; a handling definition otherwise
        virtual std::optional<Definition> Map(const std::exception_ptr& exception) const = 0;
    };

    /**
     * Mapper that maps any exception to the same handling definition
     */
    class ToConstantMapper : public Mapper
    {
        public:
            ToConstantMapper(const Definition& definition);
            virtual std::optional<Definition> Map(const std::exception_ptr& exception) const override;
        public:
            Definition _definition;
    };


    /**
     * Mapper that maps to a handling definition if an exception is of a specific type or a subclass of that type.
    */
    template<typename ExceptionType>
    class ExceptionOfTypeMapper : public ToConstantMapper
    {
        public:
            ExceptionOfTypeMapper(const Definition& definition);            
            virtual std::optional<Definition> Map(const std::exception_ptr& exception) const override;        
    };
}

/**
 * Request Hook for incoming requests that handles exceptions according to a exception handling definition:
 * - An exception can result in returning a predefined request status.
 * - Optionally the exception details can be forwarded in the status details (best practice for client side errors).
 * - Optionally a log entry with the exception details can be emitted (best practice for internal errors).
 * Note that exception details are only available for exceptions derived from std::exception.
 * The order of exception handling definitions defines the handling priority.
 * Any exception not matched by any of the exception handling definitions will be rethrown.
*/
class ExceptionHandlingRequestHook : public mse::RequestHook
{
public:        
    struct Parameters
    {
        Parameters(const std::vector<std::shared_ptr<ExceptionHandling::Mapper>>& exception_handling_mappers = ExceptionHandlingRequestHook::_default_exception_handling_mappers);
        std::vector<std::shared_ptr<ExceptionHandling::Mapper>> exception_handling_mappers = ExceptionHandlingRequestHook::_default_exception_handling_mappers;
        AutoRequestHookParameterRegistration<ExceptionHandlingRequestHook::Parameters, ExceptionHandlingRequestHook> auto_registration;
    };

    ExceptionHandlingRequestHook(const Parameters& parameters);
    virtual ~ExceptionHandlingRequestHook();    

    virtual Status Process(Func func, Context& context) override;

private:
    Parameters _parameters;    
    static const std::vector<std::shared_ptr<ExceptionHandling::Mapper>> _default_exception_handling_mappers;
};

}

#include "exception-handling-request-hook.txx"

