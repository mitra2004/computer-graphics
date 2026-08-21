"""
Experiment - Midpoint Circle Drawing Algorithm
Generates a circle on a raster display using OpenGL by exploiting the
eight-way symmetry of a circle.
"""

import glfw
from OpenGL.GL import *
import ctypes

W, H = 800, 600


def midpoint_circle(xc, yc, r):
    """Return the pixel list for a circle using the midpoint algorithm,
    exploiting eight-way symmetry (only one octant is computed)."""
    pts = []

    def plot_octants(x, y):
        pts.append((xc + x, yc + y))
        pts.append((xc - x, yc + y))
        pts.append((xc + x, yc - y))
        pts.append((xc - x, yc - y))
        pts.append((xc + y, yc + x))
        pts.append((xc - y, yc + x))
        pts.append((xc + y, yc - x))
        pts.append((xc - y, yc - x))

    x = 0
    y = r
    p = 1 - r  # initial decision parameter
    plot_octants(x, y)

    while x < y:
        x += 1
        if p < 0:
            p += 2 * x + 1
        else:
            y -= 1
            p += 2 * (x - y) + 1
        plot_octants(x, y)

    return pts


VS_SRC = """
#version 330 core
layout (location = 0) in vec2 aPos;
uniform vec2 uRes;
void main() {
    vec2 ndc = (aPos / uRes) * 2.0 - 1.0;
    gl_Position = vec4(ndc, 0.0, 1.0);
    gl_PointSize = 4.0;
}
"""

FS_SRC = """
#version 330 core
out vec4 FragColor;
uniform vec3 uColor;
void main() { FragColor = vec4(uColor, 1.0); }
"""


def compile_shader(src, kind):
    sh = glCreateShader(kind)
    glShaderSource(sh, src)
    glCompileShader(sh)
    if not glGetShaderiv(sh, GL_COMPILE_STATUS):
        raise RuntimeError(glGetShaderInfoLog(sh).decode())
    return sh


def make_program(vs_src, fs_src):
    vs = compile_shader(vs_src, GL_VERTEX_SHADER)
    fs = compile_shader(fs_src, GL_FRAGMENT_SHADER)
    prog = glCreateProgram()
    glAttachShader(prog, vs)
    glAttachShader(prog, fs)
    glLinkProgram(prog)
    if not glGetProgramiv(prog, GL_LINK_STATUS):
        raise RuntimeError(glGetProgramInfoLog(prog).decode())
    glDeleteShader(vs)
    glDeleteShader(fs)
    return prog


def main():
    if not glfw.init():
        raise RuntimeError("Failed to init GLFW")

    glfw.window_hint(glfw.CONTEXT_VERSION_MAJOR, 3)
    glfw.window_hint(glfw.CONTEXT_VERSION_MINOR, 3)
    glfw.window_hint(glfw.OPENGL_PROFILE, glfw.OPENGL_CORE_PROFILE)
    glfw.window_hint(glfw.OPENGL_FORWARD_COMPAT, GL_TRUE)

    window = glfw.create_window(W, H, "Midpoint Circle Drawing Algorithm", None, None)
    if not window:
        glfw.terminate()
        raise RuntimeError("Failed to create window")

    glfw.make_context_current(window)
    glEnable(GL_PROGRAM_POINT_SIZE)

    prog = make_program(VS_SRC, FS_SRC)
    u_res = glGetUniformLocation(prog, "uRes")
    u_color = glGetUniformLocation(prog, "uColor")

    circles = [
        (200, 300, 150),
        (600, 300, 90),
    ]

    vao = glGenVertexArrays(1)
    vbo = glGenBuffers(1)
    glBindVertexArray(vao)
    glBindBuffer(GL_ARRAY_BUFFER, vbo)
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * 4, ctypes.c_void_p(0))
    glEnableVertexAttribArray(0)
    glBindVertexArray(0)

    def upload(points):
        verts = []
        for x, y in points:
            verts.extend((float(x), float(y)))
        arr = (GLfloat * len(verts))(*verts)
        glBindBuffer(GL_ARRAY_BUFFER, vbo)
        glBufferData(GL_ARRAY_BUFFER, arr, GL_DYNAMIC_DRAW)

    while not glfw.window_should_close(window):
        if glfw.get_key(window, glfw.KEY_ESCAPE) == glfw.PRESS:
            glfw.set_window_should_close(window, True)

        glClearColor(0.05, 0.05, 0.08, 1.0)
        glClear(GL_COLOR_BUFFER_BIT)

        glUseProgram(prog)
        glUniform2f(u_res, float(W), float(H))
        glBindVertexArray(vao)

        colors = [(1.0, 0.35, 0.4), (0.4, 0.85, 1.0)]
        for (xc, yc, r), col in zip(circles, colors):
            pts = midpoint_circle(xc, yc, r)
            upload(pts)
            glUniform3f(u_color, *col)
            glDrawArrays(GL_POINTS, 0, len(pts))

        glBindVertexArray(0)
        glfw.swap_buffers(window)
        glfw.poll_events()

    glfw.terminate()


if __name__ == "__main__":
    main()
