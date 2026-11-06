// Experiment 8: glsl shaders
#include "common.h"

GLuint shaderProgram=0;
GLuint compileShader(GLenum t,const char*s){GLuint h=glCreateShader(t);glShaderSource(h,1,&s,nullptr);glCompileShader(h);return h;}
void initShaders(){const char*vs="#version 120\nattribute vec2 p; attribute vec3 c; varying vec3 v; void main(){gl_Position=vec4(p,0.,1.);v=c;}";const char*fs="#version 120\nvarying vec3 v; void main(){gl_FragColor=vec4(v,1.);}";GLuint a=compileShader(GL_VERTEX_SHADER,vs),b=compileShader(GL_FRAGMENT_SHADER,fs);shaderProgram=glCreateProgram();glAttachShader(shaderProgram,a);glAttachShader(shaderProgram,b);glBindAttribLocation(shaderProgram,0,"p");glBindAttribLocation(shaderProgram,1,"c");glLinkProgram(shaderProgram);}
void draw8(){if(!shaderProgram)initShaders();glUseProgram(shaderProgram);glBegin(GL_TRIANGLES);glVertexAttrib3f(1,1,0,0);glVertexAttrib2f(0,0,.75);glVertexAttrib3f(1,0,1,0);glVertexAttrib2f(0,-.7,-.65);glVertexAttrib3f(1,0,0,1);glVertexAttrib2f(0,.7,-.65);glEnd();glUseProgram(0);}
