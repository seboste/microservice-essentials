#include "exception-handling-helpers.h"


using namespace mse;
using namespace mse::ExceptionHandling;

ToConstantMapper::ToConstantMapper(const Definition& definition)
    : _definition(definition)
{
}

std::optional<Definition> ToConstantMapper::Map(const std::exception_ptr&) const
{
    return _definition;
}
