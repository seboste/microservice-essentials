# microservice-essentials

**_WARNING:_**  Microservice-essentials is still under development. Breaking changes can occur at any time.

**Microservice-essentials** is a portable, independent C++ library that takes care of typical recurring concerns that occur in microservice development.

However, instead of providing a framework based on a specific technology (e.g. REST or gRPC), it aims to provide technology independent functionality that facilitates the development of cloud-native microservices, yet making the integration with popular technologies as convenient as possible.

The library itself is designed to not have any dependencies on other 3rd party libraries.

## Key Concepts

### Request Handling

Stateless microservices execute some business logic when invoked by a request (e.g. HTTP). Different recurring concerns need to be addressed for each request, such as obeservability (e.g. logging, tracing, metrics), reliability (e.g. error handling, rate limiting), performance (e.g. caching), and security (e.g. token validation). **Microservice essentials** provides local (for a specific request handler) and global (for all request handlers) **hooks** that will be executed before and after executing the actual handling code. Some hooks (see below) are already provided by the library, whereas others can be implemented by the user of the library.

The following example gives an idea how this concept is implemented:
```cpp
// main.cpp registers a global hook for exception handling 
// ...
mse::RequestHandler::GloballyWith(mse::ExceptionHandlingRequestHook::Parameters{});
//...

// inside handler.cpp 
void handleSpecificRequest(const Request& req, Response& resp)
{
    resp.status = mse::ToHttpStatusCode(mse::RequestHandler("specific_request", mse::Context(mse::ToContextMetadata(request.headers)))
        .With(mse::ClaimCheckerRequestHook::ScopeContains("execute_specific_request_claim"))
        .Process([&](mse::Context&)
        {
            //... the actual handling code based on the request and response goes here
            return mse::Status();
        })
    .code);  
}
```
This example shows a handler function that is invoked for a specific request. The actual handling code is wrapped in a **RequestHandler** instance, that in this case provides an exception handler that translates exceptions into status codes and another hook that checks if the caller is authorized.

### Request Context

By definition, stateless microservices don't internally hold any kind of mutable data that is shared across individual requests. However, individual requests can have data attached to it. This is the request's **context**. The data can be provided by the caller (e.g. via http metadata) or it can be created during processing the request (e.g. a request ID). **Microservice essentials** provides the class ``mse::Context`` for holding such data in a simple string key-value store. As each request is typically handled by a single thread, the request handler stores such a context in the threadlocal storage, so that the current context can be accessed from everywhere in the callstack without explicitly passing it around. This can be handy for example when an outgoing request needs to forward data coming from the originating request (e.g. the traceparent or the authentication token).


## Features

The following sections give a rough overview of the library's content:

### Basics
- Base class for **handling** incoming traffic.
- Definition of application and request specific **context**.
- Definition of a **request status**.

### Cross Cutting Concerns
- Singleton for notifying registered classes about upcoming service **shutdown** upon receiving a shutdown signal.
- **Exception handling** for requests including optional forwarding/logging of exception details.
- **Error forwarding** from callees of the service to callers of the service.

### Observability
- A minimalistic customizeable **logging** framework including a structured logger.

### Performance
- **caching** for server and client responses.

### Reliability
- **retries** for failed outgoing requests.
- **circuit breaker** for outgoing requests.

### Request
- Global and local **hooks** that will be executed before and after handling/issuing a request.

### Security
- Base functionality for token based authentication and authorization.

### Utilities
- Convenience for reading and converting **environment variables**.
- Converting of **request status** and **context metadata**.
- Handling of **system signals**.
- Parsing of **urls**.

## Build

The library and testing should support any C++17 compiler. It has only been tested with clang & gcc on x86_64 Linux, clang on MacOS, and msvc on Windows. The examples, however, only support x86_64 Linux due to the many dependencies to 3rd party libraries.

Execute the following commands to build the library, tests, and examples:

```
mkdir build
cd build
conan install .. -s build_type=Release -o build_testing=True -o build_examples=True
cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=True -DBUILD_EXAMPLES=True
cmake --build .
```

## Tests

The library aims to have a good unit test coverage. After building them (see above), the tests can be executed with the following commands:

```
cd build/tests
ctest .
```

## Examples

The **examples** directory contains simple example microservices that are based on the **Microservice essentials** library. The examples are meant to illustrate the intended use of the library and give allows the user of the library to get started quickly.

