#pragma once

#include <string>

namespace mse
{

class Handler
{
public:
    Handler(const std::string& name = "handler");
    ~Handler();

    const std::string& getName() const { return _name; }

    virtual void Handle() = 0;
    virtual void Stop() = 0;

protected:
private:
    const std::string _name;

};

}
