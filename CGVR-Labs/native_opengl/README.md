# Native OpenGL submissions

`CGVR_OpenGLLabs.cpp` is a macOS OpenGL/GLUT program implementing the manual's experiments **1–8 and 10**. It is not a browser/WebGL substitute.

Build and run from this directory:

```sh
make
make run-1     # replace 1 with 2…8 or 10
make run-all   # displays 1–8 and 10, six seconds each, in a loop
```

The program opens a native OpenGL window. Experiments 1–5 use the requested raster/matrix algorithms; 6 uses the hardware depth buffer; 7 uses fixed-function Phong lighting/materials; 8 compiles vertex and fragment GLSL shaders; and 10 uses perspective projection, camera viewing, and viewport mapping.

Use `./cgvr_opengl all` for the automatic presentation mode. It holds each experiment for exactly 6,000 ms and then advances in the required order: 1, 2, 3, 4, 5, 6, 7, 8, 10.
