# microservice-essentials

## Features

### Basics
- Base class for **handling** incoming traffic.
- Definition of application and request specific **context**.
- Definition of a **request status**.

### Cross Cutting Concerns
- Singleton for notifying registered classes about upcoming service **shutdown** upon receiving a shutdown signal.
- **Exception handling** for requests including optional forwarding/logging of exception details.

### Observability
- A minimalistic customizeable **logging** framework including a structured logger.

### Request
- Global and local **hooks** that will be executed before and after handling/issuing a request.

### Security
- Base functionality for token based authorization.

### Utilities
- Convenience for reading and converting **environment variables**.
- Converting of **request status** and **context metadata**.
- Handling of **system signals**.

## Build
```
mkdir build
cd build
conan install .. -s build_type=Release -o build_testing=True -o build_examples=True
cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=True -DBUILD_EXAMPLES=True
cmake --build .
```

## Test
```
cd build/tests
ctest .
```
