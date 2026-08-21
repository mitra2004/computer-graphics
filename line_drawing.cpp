// =====================================================================
//  Experiment 1 - DDA and Bresenham Line Drawing Algorithms
//  OpenGL 3.3 core + GLFW + GLAD
//
//  Draws several line segments with the DDA algorithm (red) and the
//  same segments offset to the right with Bresenham's algorithm
//  (green), so the two techniques can be compared side by side.
// =====================================================================

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <vector>

struct Point { int x, y; };

// ---------------------------------------------------------------------
//  DDA (Digital Differential Analyzer)
// ---------------------------------------------------------------------
static std::vector<Point> ddaLine(int x0, int y0, int x1, int y1) {
    std::vector<Point> pts;

    int dx = x1 - x0;
    int dy = y1 - y0;
    int steps = std::max(std::abs(dx), std::abs(dy));
    if (steps == 0) { pts.push_back({x0, y0}); return pts; }

    float xInc = dx / (float)steps;
    float yInc = dy / (float)steps;

    float x = (float)x0, y = (float)y0;
    for (int i = 0; i <= steps; ++i) {
        pts.push_back({ (int)std::round(x), (int)std::round(y) });
        x += xInc;
        y += yInc;
    }
    return pts;
}

// ---------------------------------------------------------------------
//  Bresenham's Line Algorithm (all octants, integer arithmetic only)
// ---------------------------------------------------------------------
static std::vector<Point> bresenhamLine(int x0, int y0, int x1, int y1) {
    std::vector<Point> pts;

    int dx  = std::abs(x1 - x0);
    int dy  = std::abs(y1 - y0);
    int sx  = (x0 < x1) ? 1 : -1;
    int sy  = (y0 < y1) ? 1 : -1;

    int x = x0, y = y0;

    if (dx >= dy) {
        int p = 2 * dy - dx;
        for (int i = 0; i <= dx; ++i) {
            pts.push_back({x, y});
            if (p >= 0) { y += sy; p -= 2 * dx; }
            x += sx;
            p += 2 * dy;
        }
    } else {
        int p = 2 * dx - dy;
        for (int i = 0; i <= dy; ++i) {
            pts.push_back({x, y});
            if (p >= 0) { x += sx; p -= 2 * dy; }
            y += sy;
            p += 2 * dx;
        }
    }
    return pts;
}

// ---------------------------------------------------------------------
//  Minimal shader: pixel-space position (uniform resolution) + color
// ---------------------------------------------------------------------
static const char* vsSrc = R"GLSL(
#version 330 core
layout (location = 0) in vec2 aPos;
uniform vec2 uRes;
void main() {
    vec2 ndc = (aPos / uRes) * 2.0 - 1.0;
    gl_Position = vec4(ndc, 0.0, 1.0);
    gl_PointSize = 6.0;
}
)GLSL";

static const char* fsSrc = R"GLSL(
#version 330 core
out vec4 FragColor;
uniform vec3 uColor;
void main() { FragColor = vec4(uColor, 1.0); }
)GLSL";

static unsigned int compile(unsigned int type, const char* src) {
    unsigned int sh = glCreateShader(type);
    glShaderSource(sh, 1, &src, NULL);
    glCompileShader(sh);
    int ok = 0;
    glGetShaderiv(sh, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        char log[1024];
        glGetShaderInfoLog(sh, 1024, NULL, log);
        std::printf("[shader error] %s\n", log);
    }
    return sh;
}

static unsigned int makeProgram(const char* vs, const char* fs) {
    unsigned int v = compile(GL_VERTEX_SHADER, vs);
    unsigned int f = compile(GL_FRAGMENT_SHADER, fs);
    unsigned int p = glCreateProgram();
    glAttachShader(p, v);
    glAttachShader(p, f);
    glLinkProgram(p);
    glDeleteShader(v);
    glDeleteShader(f);
    return p;
}

static void uploadPoints(unsigned int vbo, const std::vector<Point>& pts) {
    std::vector<float> verts;
    verts.reserve(pts.size() * 2);
    for (auto& p : pts) { verts.push_back((float)p.x); verts.push_back((float)p.y); }
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, (long)(verts.size() * sizeof(float)), verts.data(), GL_DYNAMIC_DRAW);
}

int main() {
    if (!glfwInit()) { std::printf("Failed to init GLFW\n"); return -1; }

    const int W = 800, H = 600;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    GLFWwindow* window = glfwCreateWindow(W, H, "Exp1 - DDA vs Bresenham", NULL, NULL);
    if (!window) { std::printf("Failed to create window\n"); glfwTerminate(); return -1; }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::printf("Failed to load GLAD\n");
        glfwTerminate();
        return -1;
    }
    glViewport(0, 0, W, H);
    glEnable(GL_PROGRAM_POINT_SIZE);

    unsigned int prog = makeProgram(vsSrc, fsSrc);
    int uRes   = glGetUniformLocation(prog, "uRes");
    int uColor = glGetUniformLocation(prog, "uColor");

    unsigned int vao = 0, vbo = 0;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);

    // A few segments covering different slopes/octants.
    // DDA lines drawn on the left half, the same segments shifted +400px
    // and drawn with Bresenham on the right half for comparison.
    struct Seg { int x0, y0, x1, y1; };
    std::vector<Seg> segs = {
        { 50,  50, 300, 150 },   // slope < 1
        { 50, 100, 150, 400 },   // slope > 1
        { 50, 300, 300, 300 },   // horizontal
        { 50,  50,  50, 400 },   // vertical
        { 50, 400, 300,  50 },   // negative slope
    };

    while (!glfwWindowShouldClose(window)) {
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, 1);

        glClearColor(0.05f, 0.05f, 0.08f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(prog);
        glUniform2f(uRes, (float)W, (float)H);
        glBindVertexArray(vao);

        // DDA -> red
        glUniform3f(uColor, 1.0f, 0.3f, 0.3f);
        for (auto& s : segs) {
            auto pts = ddaLine(s.x0, s.y0, s.x1, s.y1);
            uploadPoints(vbo, pts);
            glDrawArrays(GL_POINTS, 0, (GLsizei)pts.size());
        }

        // Bresenham -> green, shifted right so both are visible at once
        glUniform3f(uColor, 0.35f, 1.0f, 0.4f);
        for (auto& s : segs) {
            auto pts = bresenhamLine(s.x0 + 400, s.y0, s.x1 + 400, s.y1);
            uploadPoints(vbo, pts);
            glDrawArrays(GL_POINTS, 0, (GLsizei)pts.size());
        }

        glBindVertexArray(0);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteProgram(prog);
    glfwTerminate();
    return 0;
}
