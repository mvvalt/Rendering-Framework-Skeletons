# Rendering-Framework-Skeletons
These are a set of application skeletons that I use for software renderers, game demos, etc.

### Third-party
OpenGL headers can be obtained from https://www.khronos.org/registry/OpenGL/index_gl.php

GLAD can be obtained from https://github.com/Dav1dde/glad

## Software rendering
### win32_legacy_opengl.cpp
I only want to display pixels on the screen, so opengl version 1.1 is (almost) good enough. I also load the wglSwapInterval extension because we really don't want vsync when we benchmark our rendering.

## OpenGL
### win32_opengl.cpp
This uses glad to create a modern opengl window.
