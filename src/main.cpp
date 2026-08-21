// =====================================================================
//  Spiral Galaxy  -  OpenGL 3.3/4.1 core  +  GLFW  +  GLAD
//
//  Controls:  drag = orbit    scroll = zoom    SPACE = pause    ESC = quit
// =====================================================================

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <cmath>
#include <cstdio>
#include <random>
#include <vector>

// ---------------------------------------------------------------------
//  Shaders
// ---------------------------------------------------------------------

// ---- Background: procedural nebula + distant starfield (fullscreen) ----
static const char* bgVS = R"GLSL(
#version 330 core
void main() {
    // Fullscreen triangle generated from gl_VertexID - no VBO needed.
    vec2 p = vec2(float((gl_VertexID << 1) & 2), float(gl_VertexID & 2));
    gl_Position = vec4(p * 2.0 - 1.0, 0.0, 1.0);
}
)GLSL";

static const char* bgFS = R"GLSL(
#version 330 core
out vec4 FragColor;

uniform vec2  uRes;
uniform float uTime;
uniform float uSquash;    // sin(tilt): how flat the disc looks on screen
uniform float uGlowScale; // world->screen scale for the core glow

float hash21(vec2 p) {
    p = fract(p * vec2(123.34, 456.21));
    p += dot(p, p + 45.32);
    return fract(p.x * p.y);
}

float vnoise(vec2 p) {
    vec2 i = floor(p);
    vec2 f = fract(p);
    f = f * f * (3.0 - 2.0 * f);
    float a = hash21(i);
    float b = hash21(i + vec2(1.0, 0.0));
    float c = hash21(i + vec2(0.0, 1.0));
    float d = hash21(i + vec2(1.0, 1.0));
    return mix(mix(a, b, f.x), mix(c, d, f.x), f.y);
}

const mat2 ROT = mat2(0.80, 0.60, -0.60, 0.80);

float fbm(vec2 p) {
    float s = 0.0;
    float a = 0.5;
    for (int i = 0; i < 6; ++i) {
        s += a * vnoise(p);
        p = ROT * p * 2.02;
        a *= 0.5;
    }
    return s;
}

void main() {
    vec2 uv = (gl_FragCoord.xy - 0.5 * uRes) / uRes.y;
    float t = uTime * 0.03;

    // --- nebula clouds (domain-warped fbm) ---
    vec2 q = uv * 1.5;
    vec2 w = vec2(fbm(q + vec2(0.0, t)), fbm(q + vec2(5.2, -t)));
    float n1 = fbm(q + 1.4 * w);
    float n2 = fbm(q * 1.7 + 3.0 * w + 7.7);

    vec3 col = vec3(0.015, 0.008, 0.038);                                  // deep space
    col += vec3(0.60, 0.07, 0.34) * pow(n1, 2.2) * 0.95;                   // magenta
    col += vec3(0.32, 0.10, 0.62) * pow(n2, 2.6) * 0.85;                   // violet
    col += vec3(0.00, 0.38, 0.46) * pow(max(n1 - n2 + 0.25, 0.0), 3.0);    // teal
    col += vec3(0.70, 0.48, 0.14) * pow(max(n2 - 0.36, 0.0), 2.0) * 0.9;   // gold dust

    // --- distant background stars on a jittered grid ---
    vec2 sp = gl_FragCoord.xy / uRes.y * 110.0;
    vec2 gi = floor(sp);
    vec2 gf = fract(sp);
    float h = hash21(gi);
    if (h > 0.955) {
        vec2 sPos = vec2(hash21(gi + 3.1), hash21(gi + 7.3));
        float d = length(gf - sPos);
        float tw = 0.65 + 0.35 * sin(uTime * 2.0 + h * 60.0);
        float s = smoothstep(0.10, 0.0, d) * tw;
        vec3 tint = mix(vec3(1.0, 0.72, 0.85), vec3(1.0, 0.92, 0.65), hash21(gi + 11.7));
        col += tint * s * 0.9;
    }

    // --- galactic core glow, squashed to match the disc tilt ---
    vec2 g = vec2(uv.x, uv.y / max(uSquash, 0.06));
    float rw = length(g) / max(uGlowScale, 1e-4);        // radius in world units
    col += vec3(1.00, 0.76, 0.38) * exp(-rw * rw / 0.030) * 0.75;  // tight gold
    col += vec3(1.00, 0.38, 0.68) * exp(-rw * rw / 0.320) * 0.22;  // wide pink halo

    col *= 1.0 - 0.35 * dot(uv, uv);                     // vignette
    col += (hash21(gl_FragCoord.xy) - 0.5) / 255.0;      // dither out the banding

    FragColor = vec4(col, 1.0);
}
)GLSL";

// ---- Stars: point sprites with differential rotation ----
static const char* starVS = R"GLSL(
#version 330 core
layout (location = 0) in vec3  aPos;
layout (location = 1) in vec3  aColor;
layout (location = 2) in float aSize;
layout (location = 3) in float aSeed;

uniform mat4  uProj;
uniform mat4  uView;
uniform float uTime;
uniform float uPointScale;

out vec3  vColor;
out float vBright;

void main() {
    // Differential rotation: inner stars orbit faster than outer ones.
    float r   = length(aPos.xz);
    float ang = uTime * 0.45 / (0.55 + 1.2 * r);
    float s   = sin(ang);
    float c   = cos(ang);
    vec3  p   = vec3(aPos.x * c - aPos.z * s, aPos.y, aPos.x * s + aPos.z * c);

    vec4 viewPos = uView * vec4(p, 1.0);
    gl_Position  = uProj * viewPos;

    float dist   = max(-viewPos.z, 0.001);
    gl_PointSize = clamp(aSize * uPointScale / dist, 1.0, 64.0);

    vColor  = aColor;
    vBright = 0.75 + 0.25 * sin(uTime * (1.5 + fract(aSeed) * 3.0) + aSeed * 30.0);
}
)GLSL";

static const char* starFS = R"GLSL(
#version 330 core
in  vec3  vColor;
in  float vBright;
out vec4  FragColor;

void main() {
    vec2  c  = gl_PointCoord * 2.0 - 1.0;
    float d2 = dot(c, c);
    if (d2 > 1.0) discard;

    float core  = pow(1.0 - d2, 4.0);
    float halo  = exp(-d2 * 4.0) * 0.35;
    float alpha = (core + halo) * vBright;

    // Push the very centre toward white so bright stars bloom.
    FragColor = vec4(vColor + core * 0.75, alpha);
}
)GLSL";

// ---------------------------------------------------------------------
//  Minimal matrix math (column-major, no GLM dependency)
// ---------------------------------------------------------------------
struct Mat4 { float m[16]; };

static Mat4 matIdentity() {
    Mat4 r{};
    r.m[0] = r.m[5] = r.m[10] = r.m[15] = 1.0f;
    return r;
}

static Mat4 matMul(const Mat4& a, const Mat4& b) {
    Mat4 r{};
    for (int c = 0; c < 4; ++c)
        for (int row = 0; row < 4; ++row) {
            float s = 0.0f;
            for (int k = 0; k < 4; ++k) s += a.m[k * 4 + row] * b.m[c * 4 + k];
            r.m[c * 4 + row] = s;
        }
    return r;
}

static Mat4 matPerspective(float fovyRad, float aspect, float zn, float zf) {
    Mat4 r{};
    float f = 1.0f / std::tan(fovyRad * 0.5f);
    r.m[0]  = f / aspect;
    r.m[5]  = f;
    r.m[10] = (zf + zn) / (zn - zf);
    r.m[11] = -1.0f;
    r.m[14] = (2.0f * zf * zn) / (zn - zf);
    return r;
}

static Mat4 matTranslate(float x, float y, float z) {
    Mat4 r = matIdentity();
    r.m[12] = x; r.m[13] = y; r.m[14] = z;
    return r;
}

static Mat4 matRotX(float a) {
    Mat4 r = matIdentity();
    float c = std::cos(a), s = std::sin(a);
    r.m[5] = c; r.m[6] = s; r.m[9] = -s; r.m[10] = c;
    return r;
}

static Mat4 matRotY(float a) {
    Mat4 r = matIdentity();
    float c = std::cos(a), s = std::sin(a);
    r.m[0] = c; r.m[2] = -s; r.m[8] = s; r.m[10] = c;
    return r;
}

// ---------------------------------------------------------------------
//  Interaction state
// ---------------------------------------------------------------------
static int    gW = 1280, gH = 800;
static float  gYaw = 0.0f;
static float  gTilt = 1.12f;          // ~64 degrees from edge-on
static float  gDist = 3.4f;
static bool   gDragging = false;
static bool   gPaused = false;
static double gLastX = 0.0, gLastY = 0.0;

static void onResize(GLFWwindow*, int w, int h) {
    gW = (w > 0) ? w : 1;
    gH = (h > 0) ? h : 1;
    glViewport(0, 0, gW, gH);
}

static void onKey(GLFWwindow* win, int key, int, int action, int) {
    if (action != GLFW_PRESS) return;
    if (key == GLFW_KEY_ESCAPE) glfwSetWindowShouldClose(win, 1);
    if (key == GLFW_KEY_SPACE)  gPaused = !gPaused;
}

static void onMouseButton(GLFWwindow* win, int button, int action, int) {
    if (button != GLFW_MOUSE_BUTTON_LEFT) return;
    gDragging = (action == GLFW_PRESS);
    glfwGetCursorPos(win, &gLastX, &gLastY);
}

static void onCursor(GLFWwindow*, double x, double y) {
    if (gDragging) {
        gYaw  += float(x - gLastX) * 0.006f;
        gTilt += float(y - gLastY) * 0.006f;
        if (gTilt < 0.05f) gTilt = 0.05f;
        if (gTilt > 1.55f) gTilt = 1.55f;
    }
    gLastX = x;
    gLastY = y;
}

static void onScroll(GLFWwindow*, double, double dy) {
    gDist *= (1.0f - float(dy) * 0.09f);
    if (gDist < 0.9f)  gDist = 0.9f;
    if (gDist > 12.0f) gDist = 12.0f;
}

// ---------------------------------------------------------------------
//  Shader helpers
// ---------------------------------------------------------------------
static unsigned int compile(unsigned int type, const char* src, const char* tag) {
    unsigned int sh = glCreateShader(type);
    glShaderSource(sh, 1, &src, NULL);
    glCompileShader(sh);
    int ok = 0;
    glGetShaderiv(sh, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        char log[2048];
        glGetShaderInfoLog(sh, 2048, NULL, log);
        std::printf("[shader compile error: %s]\n%s\n", tag, log);
    }
    return sh;
}

static unsigned int makeProgram(const char* vs, const char* fs, const char* tag) {
    unsigned int v = compile(GL_VERTEX_SHADER, vs, tag);
    unsigned int f = compile(GL_FRAGMENT_SHADER, fs, tag);
    unsigned int p = glCreateProgram();
    glAttachShader(p, v);
    glAttachShader(p, f);
    glLinkProgram(p);
    int ok = 0;
    glGetProgramiv(p, GL_LINK_STATUS, &ok);
    if (!ok) {
        char log[2048];
        glGetProgramInfoLog(p, 2048, NULL, log);
        std::printf("[program link error: %s]\n%s\n", tag, log);
    }
    glDeleteShader(v);
    glDeleteShader(f);
    return p;
}

// ---------------------------------------------------------------------
//  Galaxy generation
// ---------------------------------------------------------------------
struct V3 { float x, y, z; };

static std::mt19937 rng(20260804u);
static std::uniform_real_distribution<float> UD(0.0f, 1.0f);
static std::normal_distribution<float>       ND(0.0f, 1.0f);

static float rnd()   { return UD(rng); }
static float gauss() { return ND(rng); }

static V3 mix3(const V3& a, const V3& b, float t) {
    return V3{ a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t, a.z + (b.z - a.z) * t };
}

// Radius 0..1 -> gold core, rose, magenta, violet, ice blue rim.
static V3 paletteByRadius(float t) {
    const V3 gold    { 1.00f, 0.84f, 0.45f };
    const V3 rose    { 1.00f, 0.42f, 0.62f };
    const V3 magenta { 0.94f, 0.24f, 0.86f };
    const V3 violet  { 0.55f, 0.35f, 1.00f };
    const V3 ice     { 0.45f, 0.82f, 1.00f };

    if (t < 0.20f) return mix3(gold,    rose,    t / 0.20f);
    if (t < 0.45f) return mix3(rose,    magenta, (t - 0.20f) / 0.25f);
    if (t < 0.72f) return mix3(magenta, violet,  (t - 0.45f) / 0.27f);
    if (t < 1.00f) return mix3(violet,  ice,     (t - 0.72f) / 0.28f);
    return ice;
}

static V3 jitter(V3 c, float amt) {
    c.x += (rnd() - 0.5f) * amt;
    c.y += (rnd() - 0.5f) * amt;
    c.z += (rnd() - 0.5f) * amt;
    if (c.x < 0.0f) c.x = 0.0f;
    if (c.y < 0.0f) c.y = 0.0f;
    if (c.z < 0.0f) c.z = 0.0f;
    return c;
}

static void pushStar(std::vector<float>& v, float x, float y, float z,
                     const V3& c, float size, float seed) {
    v.push_back(x); v.push_back(y); v.push_back(z);
    v.push_back(c.x); v.push_back(c.y); v.push_back(c.z);
    v.push_back(size);
    v.push_back(seed);
}

static std::vector<float> buildGalaxy() {
    const float TAU   = 6.28318530718f;
    const int   ARMS  = 4;
    const float TWIST = 3.9f;      // radians of sweep per unit radius

    const int N_DISC  = 110000;
    const int N_BULGE = 32000;
    const int N_HALO  = 9000;

    std::vector<float> v;
    v.reserve(size_t(N_DISC + N_BULGE + N_HALO) * 8);

    // ---- spiral arms + inter-arm field stars ----
    for (int i = 0; i < N_DISC; ++i) {
        float r = 0.10f + std::pow(rnd(), 0.62f) * 0.95f;

        bool  field = (rnd() < 0.16f);
        float theta;
        if (field) {
            theta = rnd() * TAU;
        } else {
            int arm = int(rnd() * ARMS) % ARMS;
            theta = arm * (TAU / ARMS) + r * TWIST;
            theta += gauss() * (0.10f + 0.30f * r);   // arms fray outward
        }
        r += gauss() * 0.02f;
        if (r < 0.03f) r = 0.03f;

        float x = std::cos(theta) * r;
        float z = std::sin(theta) * r;
        float thickness = 0.05f * std::exp(-r * 1.6f) + 0.010f;
        float y = gauss() * thickness;

        float t = r; if (t > 1.0f) t = 1.0f;
        V3 c = jitter(paletteByRadius(t), 0.22f);
        float size = 0.5f + std::pow(rnd(), 3.0f) * 2.2f;

        if (field) {                       // field stars are dimmer and cooler
            c = mix3(c, V3{ 0.55f, 0.60f, 0.95f }, 0.35f);
            c.x *= 0.6f; c.y *= 0.6f; c.z *= 0.65f;
        } else if (rnd() < 0.0045f) {      // star-forming regions in the arms
            c = (rnd() < 0.5f) ? V3{ 1.00f, 0.30f, 0.70f }   // hot pink
                               : V3{ 1.00f, 0.86f, 0.50f };  // gold
            size = 3.5f + rnd() * 4.5f;
        }

        pushStar(v, x, y, z, c, size, rnd() * 1000.0f);
    }

    // ---- dense golden bulge ----
    for (int i = 0; i < N_BULGE; ++i) {
        float r  = std::pow(rnd(), 2.6f) * 0.34f;
        float th = rnd() * TAU;
        float ph = std::acos(2.0f * rnd() - 1.0f);

        float x = r * std::sin(ph) * std::cos(th);
        float z = r * std::sin(ph) * std::sin(th);
        float y = r * std::cos(ph) * 0.55f;      // squashed into the disc

        V3 c = mix3(V3{ 1.00f, 0.78f, 0.36f }, V3{ 1.00f, 0.97f, 0.90f }, std::pow(rnd(), 1.5f));
        c = jitter(c, 0.10f);
        float size = 0.5f + std::pow(rnd(), 2.5f) * 1.4f;
        if (rnd() < 0.002f) size = 3.0f + rnd() * 3.0f;

        pushStar(v, x, y, z, c, size, rnd() * 1000.0f);
    }

    // ---- sparse outer halo ----
    for (int i = 0; i < N_HALO; ++i) {
        float r  = 1.05f + std::pow(rnd(), 1.5f) * 1.7f;
        float th = rnd() * TAU;
        float ph = std::acos(2.0f * rnd() - 1.0f);

        float x = r * std::sin(ph) * std::cos(th);
        float z = r * std::sin(ph) * std::sin(th);
        float y = r * std::cos(ph) * 0.80f;

        V3 c = mix3(V3{ 0.50f, 0.70f, 1.00f }, V3{ 0.85f, 0.55f, 1.00f }, rnd());
        c.x *= 0.55f; c.y *= 0.55f; c.z *= 0.60f;
        pushStar(v, x, y, z, c, 0.4f + rnd() * 0.9f, rnd() * 1000.0f);
    }

    return v;
}

// ---------------------------------------------------------------------
//  main
// ---------------------------------------------------------------------
int main() {
    if (!glfwInit()) {
        std::printf("Failed to init GLFW\n");
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    GLFWwindow* window = glfwCreateWindow(gW, gH, "Galaxy", NULL, NULL);
    if (!window) {
        std::printf("Failed to create window\n");
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    glfwSetFramebufferSizeCallback(window, onResize);
    glfwSetKeyCallback(window, onKey);
    glfwSetMouseButtonCallback(window, onMouseButton);
    glfwSetCursorPosCallback(window, onCursor);
    glfwSetScrollCallback(window, onScroll);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::printf("Failed to load GLAD\n");
        glfwTerminate();
        return -1;
    }
    glfwGetFramebufferSize(window, &gW, &gH);
    glViewport(0, 0, gW, gH);

    unsigned int bgProg   = makeProgram(bgVS,   bgFS,   "background");
    unsigned int starProg = makeProgram(starVS, starFS, "stars");

    int uBgRes   = glGetUniformLocation(bgProg, "uRes");
    int uBgTime  = glGetUniformLocation(bgProg, "uTime");
    int uBgSq    = glGetUniformLocation(bgProg, "uSquash");
    int uBgGlow  = glGetUniformLocation(bgProg, "uGlowScale");

    int uProj    = glGetUniformLocation(starProg, "uProj");
    int uView    = glGetUniformLocation(starProg, "uView");
    int uTimeLoc = glGetUniformLocation(starProg, "uTime");
    int uPtScale = glGetUniformLocation(starProg, "uPointScale");

    // Core profile requires a bound VAO even for the attribute-less fullscreen pass.
    unsigned int emptyVAO = 0;
    glGenVertexArrays(1, &emptyVAO);

    std::vector<float> stars = buildGalaxy();
    const int starCount = int(stars.size() / 8);
    std::printf("Generated %d stars\n", starCount);

    unsigned int VAO = 0, VBO = 0;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER,
                 (long)(stars.size() * sizeof(float)), stars.data(), GL_STATIC_DRAW);

    const int stride = 8 * sizeof(float);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, stride, (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, stride, (void*)(7 * sizeof(float)));
    glEnableVertexAttribArray(3);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    glEnable(GL_PROGRAM_POINT_SIZE);
    glEnable(GL_MULTISAMPLE);
    glDisable(GL_DEPTH_TEST);          // additive blending: draw order doesn't matter

    const float FOV = 0.85f;           // ~49 degrees
    double prev = glfwGetTime();
    float  simTime = 0.0f;

    while (!glfwWindowShouldClose(window)) {
        double now = glfwGetTime();
        float dt = float(now - prev);
        prev = now;
        if (!gPaused) simTime += dt;

        float aspect = float(gW) / float(gH > 0 ? gH : 1);

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // ---- pass 1: nebula background ----
        glDisable(GL_BLEND);
        glUseProgram(bgProg);
        glUniform2f(uBgRes, float(gW), float(gH));
        glUniform1f(uBgTime, simTime);
        glUniform1f(uBgSq, std::sin(gTilt));
        glUniform1f(uBgGlow, (1.0f / std::tan(FOV * 0.5f)) / (2.0f * gDist));
        glBindVertexArray(emptyVAO);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        // ---- pass 2: stars, additive ----
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);

        Mat4 proj = matPerspective(FOV, aspect, 0.05f, 100.0f);
        Mat4 view = matMul(matTranslate(0.0f, 0.0f, -gDist),
                           matMul(matRotX(gTilt), matRotY(gYaw)));

        glUseProgram(starProg);
        glUniformMatrix4fv(uProj, 1, GL_FALSE, proj.m);
        glUniformMatrix4fv(uView, 1, GL_FALSE, view.m);
        glUniform1f(uTimeLoc, simTime);
        glUniform1f(uPtScale, float(gH) * 0.006f);

        glBindVertexArray(VAO);
        glDrawArrays(GL_POINTS, 0, starCount);
        glBindVertexArray(0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteVertexArrays(1, &emptyVAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(starProg);
    glDeleteProgram(bgProg);
    glfwTerminate();
    return 0;
}