#include "status-converter.h"
#include <exception>

mse::HttpStatusCodeToStatusCodeLookupTables mse::DefaultHttpStatusCodeToStatusCodeLookupTables = {
    {
        {1, mse::StatusCode::ok},               // Informational
        {2, mse::StatusCode::ok},               // Success
        {3, mse::StatusCode::ok},               // Redirection
        {4, mse::StatusCode::invalid_argument}, // Client Error
        {5, mse::StatusCode::internal}          // Server Error
    },
    {
        {401, mse::StatusCode::unauthenticated},    // Unauthorized
        {403, mse::StatusCode::permission_denied},  // Forbidden
        {404, mse::StatusCode::not_found},          // Not Found
        {409, mse::StatusCode::already_exists},     // Conflict
        {429, mse::StatusCode::resource_exhausted}, // Too Many Requests
        {501, mse::StatusCode::unimplemented},      // Not Implemented
        {503, mse::StatusCode::unavailable},        // Service Unavailable
        {504, mse::StatusCode::deadline_exceeded}   // Gateway Timeout
    }};

// from https://chromium.googlesource.com/external/github.com/grpc/grpc/+/refs/tags/v1.21.4-pre1/doc/statuscodes.md
mse::StatusCodeToHttpStatusCodeLookupTable mse::DefaultStatusCodeToHttpStatusCodeLookupTable = {
    {mse::StatusCode::ok, 200},                  // OK
    {mse::StatusCode::cancelled, 499},           // Client Closed Request (TODO: seems to be non standard)
    {mse::StatusCode::unknown, 500},             // Internal Server Error
    {mse::StatusCode::invalid_argument, 400},    // Bad Request
    {mse::StatusCode::deadline_exceeded, 504},   // Gateway Timeout
    {mse::StatusCode::not_found, 404},           // Not Found
    {mse::StatusCode::already_exists, 409},      // Conflict
    {mse::StatusCode::permission_denied, 403},   // Forbidden
    {mse::StatusCode::resource_exhausted, 429},  // Too Many Requests
    {mse::StatusCode::failed_precondition, 400}, // Bad Request
    {mse::StatusCode::aborted, 409},             // Conflict
    {mse::StatusCode::out_of_range, 400},        // Bad Request
    {mse::StatusCode::unimplemented, 501},       // Not Implemented
    {mse::StatusCode::internal, 500},            // Internal Server Error
    {mse::StatusCode::unavailable, 503},         // Service Unavailable
    {mse::StatusCode::data_loss, 500},           // Internal Server Error
    {mse::StatusCode::unauthenticated, 401},     // Unauthorized
};

mse::StatusCode mse::FromHttpStatusCode(int http_status_code, const mse::HttpStatusCodeToStatusCodeLookupTables& lut)
{
  // 1. check specific code
  auto specificStatusCodeCit = lut.second.find(http_status_code);
  if (specificStatusCodeCit != lut.second.end())
  {
    return specificStatusCodeCit->second;
  }

  // 2. check generic code
  auto genericStatusCodeCit = lut.first.find(http_status_code / 100);
  if (genericStatusCodeCit != lut.first.end())
  {
    return genericStatusCodeCit->second;
  }

  throw std::out_of_range(std::string("cannot convert http code ") + std::to_string(http_status_code) +
                          "to status code");
}

int mse::ToHttpStatusCode(const mse::StatusCode& status_code, const mse::StatusCodeToHttpStatusCodeLookupTable& lut)
{
  if (status_code == StatusCode::invalid)
  {
    throw std::invalid_argument("cannot convert StatusCode::invalid");
  }
  auto cit = lut.find(status_code);
  if (cit == lut.end())
  {
    throw std::out_of_range(std::string("cannot convert code ") + mse::to_string(status_code) + "to http status code");
  }
  return cit->second;
}