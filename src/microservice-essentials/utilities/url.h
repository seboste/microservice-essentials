#pragma once

#include <cstdint>
#include <optional>
#include <string>

namespace mse
{

/*
    struct that represents a url
    see e.g. https://en.wikipedia.org/wiki/Uniform_Resource_Identifier for details on the format
 */
struct Url
{
    std::string scheme;
    std::string userinfo;
    std::string host;
    std::string port;
    std::string path;
    std::string query;
    std::string fragment;

    std::string GetConnectionString() const; //returns "scheme://userinfo@host:port"
    std::string GetWholePath() const; //returns "/path?query#fragment"
    std::string GetString() const;
    std::optional<uint32_t> GetPort() const;    
};

bool ToUrl(const std::string& url_str, Url& url); //returns false and sets url to default in case url cannot be parsed
Url ToUrl(const std::string& url_str); //throws std::invalid_argument in case url cannot be parsed

}

namespace std
{
    inline std::string to_string(const mse::Url& url) { return url.GetString(); }
}
