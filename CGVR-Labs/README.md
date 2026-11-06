# CG & VR Lab — Native Deliverables

The native macOS OpenGL implementation for experiments 1–8 and 10 is in `native_opengl/`. It builds and runs here with Apple OpenGL and GLUT:

```sh
cd native_opengl
make
make run-1     # use run-2 through run-8, or run-10
make run-all   # shows 1–8 and 10 in order, 6 seconds per experiment
```

`Experiment09_Unity/` is a complete Unity project source for the interactive 3D scene. This Mac has no Unity editor installed, so the project has been created but cannot be opened, executed, or exported locally until a Unity editor is installed.
