# Mini OpenGL Game (LearnOpenGL-based)

This repository is a small OpenGL game project based on the LearnOpenGL examples (https://learnopengl.com). The code has been simplified and reorganized into a minimal game (`plane`) with reusable code grouped into modules for easier expansion and feature work.

**Gameplay Demo video :**
<p align="center"> <a href="https://youtu.be/fh7jHD8erLU" target="_blank"> <img src="https://img.youtube.com/vi/fh7jHD8erLU/maxresdefault.jpg" alt="Plane Demo Video" width="70%"> </a> </p>

**Current Feature**
- Modular game logic (`PlaneGame`)
- Asset management for textures and 3D objects
- CMake-based cross-platform build setup
- LearnOpenGL-based rendering utilities
- Couch co-op (Split screen) multiplayer
- In-game HUD
  - Enemy Navigation System
  - Overhead enemy health bar
  - User health bar
  - User booster energy bar
  - Red-dot
- Plane model animation
  - Flap animaiton
  - Propeller animation
- Sony Dualsense Support (**Wired Connection only**)
  - Basic control
  - Rumble effect
  - Adaptive trigger effect      

**Project Goals:**

- **Small & modular**: split runtime logic and reusable systems for easier scaling.
- **Buildable with CMake**: compatible with the original LearnOpenGL CMake flow.
- **Ready for extension**: easy to add new systems (input, AI, rendering features).

**Build through CMake command line:**

```
cd /path/to/LearnOpenGL
mkdir build && cd build
cmake ..
cmake --build .
```

**Run:**

- After a successful build the executable is placed under `bin/plane/` (on Windows CMake places binaries in `bin/plane/Debug` or `bin/plane/Release` depending on the configuration). Run the produced `plane` executable.
- For the [itch.io](https://saranunt.itch.io/bottom) version if you use windows you can start the game with `.\launcher.bat`

**Notes:**

- The project includes several third-party helpers (GLAD, STB_IMAGE, tiny Model/Shader utilities from the LearnOpenGL helper code). If prebuilt libraries in `lib/` or `dlls/` are used they may be compiler-dependent — rebuild them if you encounter link errors.
- If your IDE can't find resource files (shaders, textures, models) set the environment variable `LOGL_ROOT_PATH` to the repository root so runtime resource lookups succeed.

**Dependencies (typical)**

- **CMake**: >= 3.0
- **GLFW3**, **GLM**, **ASSIMP**, **OpenGL** development headers/libraries
- **hidApi** For interfacing with Sony Dualsense controller
- **C++17** compiler (MSVC, GCC, Clang)

**Repository Structure (top-level overview)**

```
OpenGL-Flight-Sim/
├─ CMakeLists.txt
├─ build_windows.sh
├─ cmake/
│  └─ modules/
├─ configuration/
├─ dlls/
├─ includes/
├─ lib/
├─ resources/
│  ├─ objects/
│  └─ textures/
└─ src/
   ├─ glad.c
   ├─ stb_image.cpp
   └─ plane/
      ├─ plane.cpp
      ├─ PlaneGame.h
      ├─ PlaneGame.cpp
      ├─ plane.vs
      ├─ plane.fs
      ├─ ground.vs
      └─ ground.fs
```

**Quick troubleshooting**

- If CMake cannot find a package (GLFW, GLM, ASSIMP), install the development packages for your platform or point CMake to the correct locations using `-D` options.
- On Windows: ensure `dlls/` content is copied to the runtime directory or the build target copies them (CMake in this repo will copy `dlls/*.dll` into the target folder when building on Windows).

**Credits**

- This project is based on the LearnOpenGL example code and helper utilities (https://learnopengl.com). Most helper headers and small utilities (shader, camera, model loader helpers) are included under `includes/` and `src/`.

**Authors**

- (1039) Pongpon Butseemart
- (1055) Sorrawit Udomvittayakrai
- (1092) Saranunt Park

**For Windows User**

For windows user, you can download the game from [Itch.io:BottomGunMaverick - GOTY edition](https://saranunt.itch.io/bottom)

If you'd like, I can:

- Add a short `Makefile` or PowerShell build script for Windows automation.
- Run a CMake configure/build here and fix compilation errors.
- Expand the README with development notes or a contribution guide.
