#pragma once

#include <iostream>
#include <string>

namespace mse
{

// compatible to grpc::sthttps://grpc.github.io/grpc/core/md_doc_statuscodes.html
enum class StatusCode
{
  invalid = -1,
  ok = 0,
  lowest = ok,
  cancelled = 1,
  unknown = 2,
  invalid_argument = 3,
  deadline_exceeded = 4,
  not_found = 5,
  already_exists = 6,
  permission_denied = 7,
  resource_exhausted = 8,
  failed_precondition = 9,
  aborted = 10,
  out_of_range = 11,
  unimplemented = 12,
  internal = 13,
  unavailable = 14,
  data_loss = 15,
  unauthenticated = 16,
  highest = unauthenticated
};

/**
 * Simple status code struct. Use utilities/status-converter to convert from/to grpc/http error codes
 */
struct Status
{
  StatusCode code = StatusCode::ok;
  std::string details;

  operator bool() const
  {
    return code == StatusCode::ok;
  }

  static Status OK;
};

std::string to_string(StatusCode status_code);
void from_string(const std::string& status_code_string, StatusCode& status_code);

} // namespace mse

// stream operators for StatusCode
bool operator>>(std::istream& is, mse::StatusCode& status_code);
std::ostream& operator<<(std::ostream& os, const mse::StatusCode& status_code);