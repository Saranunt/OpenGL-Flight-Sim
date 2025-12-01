# Mini OpenGL Game (LearnOpenGL-based)

This repository is a small OpenGL game project based on the LearnOpenGL examples (https://learnopengl.com). The code has been simplified and reorganized into a minimal game (`plane`) with reusable code grouped into modules for easier expansion and feature work.

**Current Progress:**

https://github.com/user-attachments/assets/81037af2-66e3-45c2-aec4-d366b0dd8fa7

**What's Next?:**
- Frame Buffer for dual screen multiplayer
- better plane animation
- better map/ sky box
- joy stick control

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

**Notes:**

- The project includes several third-party helpers (GLAD, STB_IMAGE, tiny Model/Shader utilities from the LearnOpenGL helper code). If prebuilt libraries in `lib/` or `dlls/` are used they may be compiler-dependent — rebuild them if you encounter link errors.
- If your IDE can't find resource files (shaders, textures, models) set the environment variable `LOGL_ROOT_PATH` to the repository root so runtime resource lookups succeed.

**Dependencies (typical)**

- **CMake**: >= 3.0
- **GLFW3**, **GLM**, **ASSIMP**, **OpenGL** development headers/libraries
- **C++17** compiler (MSVC, GCC, Clang)

**Repository Structure (top-level overview)**

- `CMakeLists.txt`: top-level build script
- `bin/` : build output (created by CMake)
- `cmake/` : CMake helper modules
- `dlls/` : optional windows DLLs for runtime
- `includes/` : third-party headers delivered with the repo
- `lib/` : optional prebuilt libraries
- `resources/`
  - `objects/` : 3D models used by the game
  - `textures/` : textures used by the game
- `src/`
  - `glad.c`, `stb_image.cpp` : helper sources built as libraries
  - `plane/` : the active game target
    - `plane.cpp` : program entry (initializes GLFW/GLAD and runs game loop)
    - `PlaneGame.h`, `PlaneGame.cpp` : game module (plane state, islands, ground, rendering)
    - `plane.vs`, `plane.fs`, `ground.vs`, `ground.fs` : shader files used by the game

**Quick troubleshooting**

- If CMake cannot find a package (GLFW, GLM, ASSIMP), install the development packages for your platform or point CMake to the correct locations using `-D` options.
- On Windows: ensure `dlls/` content is copied to the runtime directory or the build target copies them (CMake in this repo will copy `dlls/*.dll` into the target folder when building on Windows).

**Credits**

- This project is based on the LearnOpenGL example code and helper utilities (https://learnopengl.com). Most helper headers and small utilities (shader, camera, model loader helpers) are included under `includes/` and `src/`.

If you'd like, I can:

- Add a short `Makefile` or PowerShell build script for Windows automation.
- Run a CMake configure/build here and fix compilation errors.
- Expand the README with development notes or a contribution guide.
