// Shared headers and helpers for the native OpenGL/GLUT experiments (1-8, 10).
#pragma once
#include <OpenGL/gl.h>
#include <OpenGL/glext.h>
#include <GLUT/glut.h>
#include <algorithm>
#include <array>
#include <cmath>
#include <cstring>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

struct P { float x,y,z; };
void polygon(const std::vector<P>&q);
void draw1(); void draw2(); void draw3(); void draw4(); void draw5();
void draw6(); void draw7(); void draw8(); void draw10();
