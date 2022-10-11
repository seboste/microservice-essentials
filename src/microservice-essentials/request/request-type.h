#pragma once

#include <iostream>
#include <string>

namespace mse
{

enum class RequestType
{
    invalid = -1,
    incoming,
    outgoing
};

std::string to_string(RequestType request_type);
void from_string(const std::string& request_type_string, RequestType& request_type);

}

//stream operators for RequestType
bool operator>>(std::istream& is, mse::RequestType& request_type);
std::ostream& operator<<(std::ostream& os, const mse::RequestType& request_type);
