// Simple OpenGL demo: DDA and Bresenham line drawing
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <cmath>
#include <cstdio>
#include <vector>
#include <chrono>
#include <random>
#include <iostream>

static int gW = 800, gH = 600;
static bool useBresen = false;
static bool dashed = false;
static bool dotted = false;
static int thickness = 1;
static bool animate = false;

static void onResize(GLFWwindow*, int w, int h) {
    gW = (w>0)?w:1; gH = (h>0)?h:1;
    glViewport(0,0,gW,gH);
}

static void onKey(GLFWwindow* w, int key, int, int action, int) {
    if (action != GLFW_PRESS) return;
    if (key == GLFW_KEY_ESCAPE) glfwSetWindowShouldClose(w, 1);
    if (key == GLFW_KEY_1) useBresen = false;
    if (key == GLFW_KEY_2) useBresen = true;
    if (key == GLFW_KEY_D) dashed = !dashed;
    if (key == GLFW_KEY_O) dotted = !dotted;
    if (key == GLFW_KEY_T) thickness = (thickness % 5) + 1;
    if (key == GLFW_KEY_A) animate = !animate;
    if (key == GLFW_KEY_C) {
        // timing comparison
        std::mt19937 rng(12345);
        std::uniform_int_distribution<int> udx(0, gW-1), udy(0, gH-1);
        const int N = 1000;
        std::vector<std::pair<int,int>> lines; lines.reserve(N*2);
        for (int i=0;i<N;i++) lines.emplace_back(udx(rng), udy(rng)), lines.emplace_back(udx(rng), udy(rng));
        auto timeit = [&](bool bres){
            auto t0 = std::chrono::high_resolution_clock::now();
            for (int i=0;i<N;i++) {
                int x0 = lines[2*i].first, y0 = lines[2*i].second;
                int x1 = lines[2*i+1].first, y1 = lines[2*i+1].second;
                std::vector<std::pair<int,int>> pts;
                if (bres) {
                    // call bres
                    int dx = std::abs(x1-x0), sx = x0<x1?1:-1;
                    int dy = -std::abs(y1-y0), sy = y0<y1?1:-1;
                    int err = dx+dy;
                    while (true) {
                        pts.emplace_back(x0,y0);
                        if (x0==x1 && y0==y1) break;
                        int e2 = 2*err;
                        if (e2 >= dy) { err += dy; x0 += sx; }
                        if (e2 <= dx) { err += dx; y0 += sy; }
                    }
                } else {
                    int dx = x1-x0; int dy = y1-y0;
                    int steps = std::max(std::abs(dx), std::abs(dy));
                    float xi = dx/(float)steps; float yi = dy/(float)steps;
                    float x = x0, y = y0;
                    for (int s=0;s<=steps;s++) { pts.emplace_back(int(std::round(x)), int(std::round(y))); x+=xi; y+=yi; }
                }
            }
            auto t1 = std::chrono::high_resolution_clock::now();
            return std::chrono::duration<double>(t1-t0).count();
        };
        double d1 = timeit(false);
        double d2 = timeit(true);
        std::printf("DDA time for 1000 lines: %.6fs\nBresenham time: %.6fs\n", d1, d2);
    }
}

static void compileCheck(unsigned int s, const char* tag) {
    int ok=0; glGetShaderiv(s, GL_COMPILE_STATUS, &ok); if (!ok) { char log[1024]; glGetShaderInfoLog(s, 1024, NULL, log); std::printf("Shader %s compile error:\n%s\n", tag, log); }
}

static std::vector<std::pair<int,int>> dda(int x0,int y0,int x1,int y1) {
    std::vector<std::pair<int,int>> pts;
    int dx = x1 - x0, dy = y1 - y0;
    int steps = std::max(std::abs(dx), std::abs(dy));
    if (steps == 0) { pts.emplace_back(x0,y0); return pts; }
    float xi = dx / (float) steps;
    float yi = dy / (float) steps;
    float x = x0, y = y0;
    for (int i=0;i<=steps;i++) { pts.emplace_back(int(std::round(x)), int(std::round(y))); x += xi; y += yi; }
    return pts;
}

static std::vector<std::pair<int,int>> bresenham(int x0,int y0,int x1,int y1) {
    std::vector<std::pair<int,int>> pts;
    int dx = std::abs(x1-x0), sx = x0<x1?1:-1;
    int dy = -std::abs(y1-y0), sy = y0<y1?1:-1;
    int err = dx + dy;
    while (true) {
        pts.emplace_back(x0,y0);
        if (x0==x1 && y0==y1) break;
        int e2 = 2*err;
        if (e2 >= dy) { err += dy; x0 += sx; }
        if (e2 <= dx) { err += dx; y0 += sy; }
    }
    return pts;
}

static void addThick(std::vector<std::pair<int,int>>& out, int cx, int cy, int t) {
    int r = t/2;
    for (int dx=-r; dx<=r; ++dx) for (int dy=-r; dy<=r; ++dy) out.emplace_back(cx+dx, cy+dy);
}

static void to_ndc(float x, float y, float &nx, float &ny) {
    nx = (x + 0.5f) / (float)gW * 2.0f - 1.0f;
    ny = 1.0f - (y + 0.5f) / (float)gH * 2.0f;
}

int main() {
    if (!glfwInit()) return -1;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
    GLFWwindow* win = glfwCreateWindow(gW, gH, "Line Drawing (DDA / Bresenham)", NULL, NULL);
    if (!win) { glfwTerminate(); return -1; }
    glfwMakeContextCurrent(win);
    glfwSetFramebufferSizeCallback(win, onResize);
    glfwSetKeyCallback(win, onKey);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) { std::printf("Failed to load GLAD\n"); return -1; }

    const char* vs = R"GLSL(#version 330 core
layout(location=0) in vec2 aPos;
layout(location=1) in vec3 aColor;
out vec3 vColor;
void main(){ gl_Position = vec4(aPos,0.0,1.0); vColor = aColor; }
)GLSL";

    const char* fs = R"GLSL(#version 330 core
in vec3 vColor; out vec4 FragColor; void main(){ FragColor = vec4(vColor,1.0); })GLSL";

    unsigned int vsS = glCreateShader(GL_VERTEX_SHADER); glShaderSource(vsS,1,&vs,NULL); glCompileShader(vsS); compileCheck(vsS,"vs");
    unsigned int fsS = glCreateShader(GL_FRAGMENT_SHADER); glShaderSource(fsS,1,&fs,NULL); glCompileShader(fsS); compileCheck(fsS,"fs");
    unsigned int prog = glCreateProgram(); glAttachShader(prog,vsS); glAttachShader(prog,fsS); glLinkProgram(prog);
    glDeleteShader(vsS); glDeleteShader(fsS);

    unsigned int VAO, VBO; glGenVertexArrays(1,&VAO); glGenBuffers(1,&VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    // layout: vec2 pos, vec3 color
    glVertexAttribPointer(0,2,GL_FLOAT,GL_FALSE,5*sizeof(float),(void*)0); glEnableVertexAttribArray(0);
    glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,5*sizeof(float),(void*)(2*sizeof(float))); glEnableVertexAttribArray(1);
    glBindVertexArray(0);

    // One DDA line (red) and one Bresenham line (green), offset so both are visible.
    struct Seg { int x0,y0,x1,y1; bool bres; float r,g,b; };
    std::vector<Seg> segs = {
        { 50,  50, 750, 350, false, 1.0f, 0.3f, 0.3f },  // DDA
        { 50, 450, 750, 150, true,  0.35f, 1.0f, 0.4f }, // Bresenham
    };

    double t0 = glfwGetTime();
    float progress = 1.0f;
    while (!glfwWindowShouldClose(win)) {
        double t1 = glfwGetTime();
        float dt = float(t1 - t0); t0 = t1;
        if (animate) progress = fmod(progress + dt*0.5f, 1.0f);

        std::vector<float> verts; verts.reserve(100000);
        for (auto& s : segs) {
            std::vector<std::pair<int,int>> pts = s.bres ? bresenham(s.x0,s.y0,s.x1,s.y1) : dda(s.x0,s.y0,s.x1,s.y1);
            if (dashed || dotted) {
                std::vector<std::pair<int,int>> filt; filt.reserve(pts.size());
                int pattern = dashed ? 12 : 4;
                for (size_t k=0;k<pts.size();++k) {
                    if (dotted) { if (k % pattern == 0) filt.push_back(pts[k]); }
                    else { if ((k % pattern) < (pattern*2/3)) filt.push_back(pts[k]); }
                }
                pts.swap(filt);
            }
            size_t drawCount = pts.size();
            if (animate) drawCount = size_t(pts.size() * progress);
            for (size_t k=0;k<drawCount;k++) {
                if (thickness <= 1) {
                    float nx, ny; to_ndc((float)pts[k].first, (float)pts[k].second, nx, ny);
                    verts.push_back(nx); verts.push_back(ny);
                    verts.push_back(s.r); verts.push_back(s.g); verts.push_back(s.b);
                } else {
                    std::vector<std::pair<int,int>> tmp; addThick(tmp, pts[k].first, pts[k].second, thickness);
                    for (auto &p: tmp) { float nx,ny; to_ndc((float)p.first,(float)p.second,nx,ny); verts.push_back(nx); verts.push_back(ny); verts.push_back(s.r); verts.push_back(s.g); verts.push_back(s.b); }
                }
            }
        }

        glClearColor(0.07f,0.07f,0.08f,1.0f); glClear(GL_COLOR_BUFFER_BIT);
        if (!verts.empty()) {
            glBindBuffer(GL_ARRAY_BUFFER, VBO);
            glBufferData(GL_ARRAY_BUFFER, verts.size()*sizeof(float), verts.data(), GL_DYNAMIC_DRAW);
            glUseProgram(prog);
            glBindVertexArray(VAO);
            glDrawArrays(GL_POINTS, 0, int(verts.size()/5));
            glBindVertexArray(0);
        }

        glfwSwapBuffers(win);
        glfwPollEvents();
    }

    glDeleteBuffers(1,&VBO); glDeleteVertexArrays(1,&VAO); glDeleteProgram(prog);
    glfwTerminate();
    return 0;
}
