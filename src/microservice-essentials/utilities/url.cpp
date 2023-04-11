#include "url.h"
#include <regex>
#include <stdexcept>

using namespace mse;

std::string Url::GetConnectionString() const
{
    std::string connection_string = host;
    if(!userinfo.empty())
    {
        connection_string = userinfo + "@" + connection_string;
    }
    if(!scheme.empty())
    {
        connection_string = scheme + "://" + connection_string;
    }
    if(!port.empty())
    {
        connection_string = connection_string + ":" + port;
    }
    return connection_string;
}


std::string Url::GetWholePath() const
{
    std::string whole_path = "/";
    if(!path.empty())
    {
        whole_path = whole_path + path;
    }
    if(!query.empty())
    {
        whole_path = whole_path + "?" + query;        
    }
    if(!fragment.empty())
    {
        whole_path = whole_path + "#" + fragment;
    }
    return whole_path;
}

std::string Url::GetString() const
{
    return GetConnectionString() + GetWholePath();
}

std::optional<uint32_t> Url::GetPort() const
{
    if(port.empty())
    {
        return std::nullopt;
    }    
    return std::stoi(port);
}

bool mse::ToUrl(const std::string& url_str, Url& url)
{
    url = Url();
    static const std::regex url_regex("((.*)(:\\/\\/))?((.+)@)?([^\\/\\?#:]+)(\\:(\\d+))?(\\/([^\\?#]*))?(\\?([^#]+))?(#(.+))?");

    std::smatch match;
    std::regex_match(url_str, match, url_regex);
    if(match.size() != 15)
    {        
        return false;
    }

    url.scheme      = match[2];
    url.userinfo    = match[5];
    url.host        = match[6];
    url.port        = match[8];
    url.path        = match[10];
    url.query       = match[12];
    url.fragment    = match[14];
    
    return true;
}

Url mse::ToUrl(const std::string& url_str)
{
    Url url;
    if(!ToUrl(url_str, url))
    {
        throw std::invalid_argument("invalid url format");
    }
    return url;
}
