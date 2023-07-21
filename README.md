
# Readme for QuickGL

Copyright (C) 2023 Marek Zalewski aka Drwalin

QuickGL is highly performant opengl graphics language. It's suited for games
with very simple graphics and simple linear animations without animation
blending.

Engine is tested to work well on Intel (with integraded gpu) platform with
suppored extensions listed in architecture/OpenGLExtensions.txt

# Features

## Supported Features

- Frustum Culling
- Linear animations - animation loading is very strict as of now
- Multiple cameras

## Features to be likely supported in near future

- Occlusion Culling
- Post processing
- Deffered shading:
    - Lights
    - Shadows
    - Glow
- Textures
- Texture atlas
- Reflections
- Level Of Detail
    - Loaded from file
    - Automatically generated

## Unsuppored Features

- Animation Blending
- Ray Tracing


# Compilation

## Required libraries (linux)

- GL
- GLU
- glfw
- GLEW
- x11
- Xxf86vm
- Xrandr
- Xi

## Required libraries (windows)

- opengl32
- glfw3
- glew32
- glu32

## Required include files from libraries

- `<GL/glew.h>`
- `<GL/glew.h>`
- `<GLFW/glfw3.h>`
- `<glm/glm.hpp>`
- `<glm/gtc/matrix_transform.hpp>`
- `<glm/gtc/type_ptr.hpp>`
- `<glm/gtc/quaternion.hpp>`
- `<glm/matrix.hpp>`

## Required C++ version

Compilator able to build with C++17 standard

