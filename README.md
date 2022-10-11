# microservice-essentials

## Features

### Basics
- Base class for **handling** incoming traffic.
- Definition of application and request specific **context**.

### Cross Cutting Concerns
- Singleton for notifying registered classes about upcoming service **shutdown** upon receiving a shutdown signal.

### Observability
- A minimalistic customizeable **logging** framework including a structured logger.

### Request
- global and local **Hooks** that will be executed before and after processing/issuing a request.

### Utilities
- Convenience for reading and converting **environment variables**.
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
