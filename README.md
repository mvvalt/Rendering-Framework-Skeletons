# Rendering-Framework-Skeletons
These are a set of application skeletons that I use for software renderers, game demos, etc.


## Software rendering
### win32_legacy_opengl.cpp
We only want to display our pixels on the screen, so opengl version 1.1 is (almost) good enough. We also load the wglSwapInterval extension because we really don't want vsync when we benchmark our rendering.
