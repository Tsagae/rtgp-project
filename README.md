# Real-Time Graphics Programming Project

The project consists in implementing a disintegrating mesh particle effect

![demo](https://github.com/user-attachments/assets/0cac9e11-72f0-4d36-9f9e-52678fb7db9b)

## Rendering Steps

1. Drawing the mesh to the main framebuffer but discarding fragments that have a value lower than a (gradually
   increasing) threshold on a mask texture
2. Draw the mesh but only the discarded fragments to a separate off-screen framebuffer
3. Read the off-screen framebuffer on cpu and spawn particles at the position of the discarded fragments
4. Update particles
5. Draw particles with instancing

## Build

### Windows

#### Dependencies

Install git and cmake

Install [build tools for visual studio](https://visualstudio.microsoft.com/downloads/#build-tools-for-visual-studio-2022)
and check optional during installation:

- msvc v143
- c++ cmake tools for windows
- Windows 10 sdk

#### Compilation

Open cmd with "Launch" in visual studio build tools 2022 or from start "Developer powershell for VS 2022" and in the
root folder of the project run:

```shell
   git clone https://github.com/microsoft/vcpkg.git
   cd vcpkg; .\bootstrap-vcpkg.bat -disableMetrics
   cmake -DCMAKE_BUILD_TYPE=Release --preset=default
   cmake --build .\build
```

To not build the benchmark executable add `-DBUILD_BENCHMARK=OFF` to this cmake command:
```shell
   cmake -DCMAKE_BUILD_TYPE=Release -DBUILD_BENCHMARK=OFF --preset=default
```

### Linux

#### Dependencies

- Ubuntu

```shell
git cmake g++ libglfw3-dev libassimp-dev libglm-dev libbenchmark-dev
```

- Fedora

```shell
git cmake g++ glfw-devel assimp-devel glm-devel google-benchmark-devel
```

#### Compilation

- Debug build

```shell
  cmake -DCMAKE_BUILD_TYPE=Debug -G "Unix Makefiles" -S . -B ./cmake-build-debug
  cmake --build ./cmake-build-debug -- -j 10
```

- Release build

```shell
  cmake -DCMAKE_BUILD_TYPE=Release -G "Unix Makefiles" -S . -B ./cmake-build-release
  cmake --build ./cmake-build-release -- -j 10
```

To not build the benchmark executable add `-DBUILD_BENCHMARK=OFF` to this cmake command:
```shell
   cmake -DCMAKE_BUILD_TYPE=Release -DBUILD_BENCHMARK=OFF -G "Unix Makefiles" -S . -B ./cmake-build-release
```

## Running

Models and textures can be added respectively to assets/models and assets/textures and will automatically appear in the menu after a restart of the application

### Options

- `RTGP-Project [width] [height]` - Run the program with a custom resolution (without arguments defaults to 1920x1080)

### Controls

- `Esc` - Toggles the menu on and off
- `R` - Resets the simulation
- `P` - Pauses the simulation
- `Mouse` - Moving the mouse while the menu is closed moves the model on the XY plane

### Menu Features

- Change model and texture
- Change mask texture used to make the model disappear
- Rotate and scale the model
- Particle control (max number, size, speed, lifetime, direction, movement randomness)

## Resources
- Inspiration for this project [Disintegrating Meshes with Particles in 'God of War' GDC 2019 Talk](https://youtu.be/ajNSrTprWsg)
- [Particles and instancing](http://www.opengl-tutorial.org/intermediate-tutorials/billboards-particles/particles-instancing/)
- [Render to texture](http://www.opengl-tutorial.org/intermediate-tutorials/tutorial-14-render-to-texture/)
- [PBO](https://www.songho.ca/opengl/gl_pbo.html)
- [Opengl guides](https://www.learnopengl.com)
- [RTGP Course material](https://www.unimi.it/en/education/degree-programme-courses/2025/real-time-graphics-programming)
- Credits to [Screaming Brain Studios](https://screamingbrainstudios.com) for most of the noise textures
