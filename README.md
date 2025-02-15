## Build

### Windows
Install git and cmake

Install [build tools for visual studio](https://visualstudio.microsoft.com/downloads/#build-tools-for-visual-studio-2022) and check optional during installation:
- msvc v143
- c++ cmake tools for windows
- Windows 10 sdk

open cmd with "Launch" in visual studio build tools 2022 or from start "Developer powershell for VS 2022" and in the root folder of the project run:
```
git clone https://github.com/microsoft/vcpkg.git
cd vcpkg; .\bootstrap-vcpkg.bat -disableMetrics
cmake --preset=default
cmake --build .\build --config Release
```

### Linux

#### Dependencies
- Ubuntu
```
git cmake g++ libglfw3-dev libassimp-dev libglm-dev
```
- Fedora
```
git cmake g++ glfw-devel assimp-devel glm-devel
```

#### Compilation

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
