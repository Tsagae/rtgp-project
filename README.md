## Build
### Linux

*TODO: Add dependencies*

- Debug build
```shell
  cmake -DCMAKE_BUILD_TYPE=Debug -G "Unix Makefiles" -S . -B ./cmake-build-debug
  cmake --build ./cmake-build-debug
```
- Release build
```shell
  cmake -DCMAKE_BUILD_TYPE=Release -G "Unix Makefiles" -S . -B ./cmake-build-release
  cmake --build ./cmake-build-release
```