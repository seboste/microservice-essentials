#pragma once

#include <exception>
#include <microservice-essentials/observability/logger.h>
#include <microservice-essentials/status.h>
#include <optional>

namespace mse
{

namespace ExceptionHandling
{
/**
 * Struct that defines how an exception shall be handled
 */
struct Definition
{
  mse::Status status;                               // status to be returned by the request
  mse::LogLevel log_level = mse::LogLevel::invalid; // do not log by default
  bool forward_exception_details_to_caller = false; // do not leak private details by default.
};

/**
 * Base class for mappers that map an exception to a handling definition
 */
class Mapper
{
public:
  virtual ~Mapper() = default;

  // shall return nullopt_t in case the exception type is not handled; a handling definition otherwise
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
template <typename ExceptionType> class ExceptionOfTypeMapper : public ToConstantMapper
{
public:
  ExceptionOfTypeMapper(const Definition& definition);
  virtual std::optional<Definition> Map(const std::exception_ptr& exception) const override;
};
} // namespace ExceptionHandling
} // namespace mse

#include "exception-handling-helpers.txx"
