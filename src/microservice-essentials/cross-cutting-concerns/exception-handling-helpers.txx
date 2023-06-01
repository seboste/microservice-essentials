#include "exception-handling-request-hook.h"

template <typename ExceptionType>
mse::ExceptionHandling::ExceptionOfTypeMapper<ExceptionType>::ExceptionOfTypeMapper(
    const mse::ExceptionHandling::Definition& definition)
    : mse::ExceptionHandling::ToConstantMapper(definition)
{
}

template <typename ExceptionType>
std::optional<mse::ExceptionHandling::Definition> mse::ExceptionHandling::ExceptionOfTypeMapper<ExceptionType>::Map(
    const std::exception_ptr& exception) const
{
  try
  {
    std::rethrow_exception(exception);
  }
  catch (const ExceptionType&)
  {
    return ToConstantMapper::Map(exception);
  }
  catch (...)
  {
    return std::nullopt;
  }
}
