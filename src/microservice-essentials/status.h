#pragma once

#include <string>

namespace mse
{ 

//compatible to grpc::sthttps://grpc.github.io/grpc/core/md_doc_statuscodes.html
enum class StatusCode
{
    ok = 0,
    cancelled = 1,
    unknown = 2,
    invalid_argument = 3,
    deadline_exceeded  = 4,
    not_found = 5,
    already_exists = 6,
    permission_deinied = 7,
    resource_exhausted = 8,
    failed_precondition = 9,
    aborted = 10,
    out_of_range = 11,
    unimplemented = 12,
    internal = 13,
    unavailable = 14,
    data_loss = 15,
    unauthenticated = 16
};

struct Status
{    
    StatusCode code = StatusCode::ok;
    std::string details;

    operator bool() const { return code == StatusCode::ok; }
};

std::string to_string(StatusCode status_code);
void from_string(const std::string& status_code_string, StatusCode& status_code);

}
