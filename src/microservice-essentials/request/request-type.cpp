#include "request-type.h"
#include <stdexcept>
#include <unordered_map>

using namespace mse;

std::string mse::to_string(RequestType request_type)
{
  switch (request_type)
  {
  case RequestType::incoming:
    return "INCOMING";
  case RequestType::outgoing:
    return "OUTGOING";
  case RequestType::invalid:
    break;
  }
  throw std::invalid_argument(std::string("invalid request type with id ") +
                              std::to_string(static_cast<int>(request_type)));
}

void mse::from_string(const std::string& request_type_string, RequestType& request_type)
{
  static const std::unordered_map<std::string, RequestType> mapping = {{"INCOMING", RequestType::incoming},
                                                                       {"OUTGOING", RequestType::outgoing}};

  auto cit = mapping.find(request_type_string);
  request_type = (cit != mapping.cend()) ? cit->second : RequestType::invalid;
}

bool operator>>(std::istream& is, mse::RequestType& request_type)
{
  request_type = RequestType::invalid;
  std::string request_type_string;
  is >> request_type_string;
  mse::from_string(request_type_string, request_type);
  return request_type != RequestType::invalid;
}

std::ostream& operator<<(std::ostream& os, const mse::RequestType& request_type)
{
  os << mse::to_string(request_type);
  return os;
}
