# microservice-essentials

## Build
```
mkdir build
cd build
conan install .. -s build_type=Release -o build_testing=True -o build_examples=True
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build .
```