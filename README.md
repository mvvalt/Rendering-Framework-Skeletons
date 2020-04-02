# Rendering-Framework-Skeletons
These are a set of application skeletons that I use for software renderers, game demos, emulators, etc.

### Third-party
OpenGL headers can be obtained from https://www.khronos.org/registry/OpenGL/index_gl.php

glad can be obtained from https://github.com/Dav1dde/glad

## win32_gdi.cpp
Display a pixel buffer on the screen using StretchDIBits().

## win32_legacy_opengl.cpp
I only want to display pixels on the screen, so opengl version 1.1 is (almost) good enough. I also load the wglSwapInterval() function to toggle vsync.

## win32_opengl.cpp
This uses glad to create a modern opengl window.

## sdl2_main.cpp
A basic game/emulation loop implemented in SDL2.
