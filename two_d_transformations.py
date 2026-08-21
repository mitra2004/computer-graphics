"""
EXPERIMENT - 3
Implementation of Two-Dimensional Transformations Using Homogeneous Coordinates

Aim
To implement the fundamental two-dimensional geometric transformations
(translation, rotation, scaling and shearing) using homogeneous coordinate
representation and transformation matrices in OpenGL.
"""

import glfw
from OpenGL.GL import *
import ctypes
import math

W, H = 900, 700


# ---------------------------------------------------------------------------
# Homogeneous-coordinate transformation matrices (3x3, row-major).
# A 2D point (x, y) is represented as the homogeneous triple (x, y, 1).
# Transformed point = M . [x, y, 1]^T
# ---------------------------------------------------------------------------

def identity_matrix():
    return [[1, 0, 0],
            [0, 1, 0],
            [0, 0, 1]]


def translation_matrix(tx, ty):
    return [[1, 0, tx],
            [0, 1, ty],
            [0, 0, 1]]


def rotation_matrix(theta_deg, cx=0, cy=0):
    """Rotate by theta_deg (counter-clockwise) about the pivot (cx, cy)."""
    t = math.radians(theta_deg)
    cos_t, sin_t = math.cos(t), math.sin(t)
    # Translate pivot to origin, rotate, translate back.
    to_origin = translation_matrix(-cx, -cy)
    rotate = [[cos_t, -sin_t, 0],
              [sin_t,  cos_t, 0],
              [0,       0,    1]]
    back = translation_matrix(cx, cy)
    return matmul(back, matmul(rotate, to_origin))


def scaling_matrix(sx, sy, cx=0, cy=0):
    """Scale by (sx, sy) about the pivot (cx, cy)."""
    to_origin = translation_matrix(-cx, -cy)
    scale = [[sx, 0,  0],
             [0,  sy, 0],
             [0,  0,  1]]
    back = translation_matrix(cx, cy)
    return matmul(back, matmul(scale, to_origin))


def shearing_matrix(shx, shy):
    return [[1,   shx, 0],
            [shy, 1,   0],
            [0,   0,   1]]


def matmul(a, b):
    """Multiply two 3x3 matrices."""
    result = [[0, 0, 0] for _ in range(3)]
    for i in range(3):
        for j in range(3):
            result[i][j] = sum(a[i][k] * b[k][j] for k in range(3))
    return result


def apply_transform(matrix, points):
    """Apply a 3x3 homogeneous matrix to a list of (x, y) points."""
    transformed = []
    for x, y in points:
        px = matrix[0][0] * x + matrix[0][1] * y + matrix[0][2] * 1
        py = matrix[1][0] * x + matrix[1][1] * y + matrix[1][2] * 1
        transformed.append((px, py))
    return transformed


# ---------------------------------------------------------------------------
# Rendering (GLFW + modern OpenGL, matching the style of midpoint_circle.py)
# ---------------------------------------------------------------------------

VS_SRC = """
#version 330 core
layout (location = 0) in vec2 aPos;
uniform vec2 uRes;
void main() {
    vec2 ndc = (aPos / uRes) * 2.0 - 1.0;
    gl_Position = vec4(ndc, 0.0, 1.0);
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

    window = glfw.create_window(
        W, H, "2D Transformations Using Homogeneous Coordinates", None, None
    )
    if not window:
        glfw.terminate()
        raise RuntimeError("Failed to create window")

    glfw.make_context_current(window)

    prog = make_program(VS_SRC, FS_SRC)
    u_res = glGetUniformLocation(prog, "uRes")
    u_color = glGetUniformLocation(prog, "uColor")

    # Base shape: a triangle, defined around a local pivot.
    base_shape = [(0, 60), (-50, -40), (50, -40)]

    # Each entry: (label placeholder, pivot position on screen, matrix, color)
    center = (150, 500)
    scenes = [
        # Original shape, drawn in place.
        ("original", identity_matrix(), (150, 500), (0.9, 0.9, 0.9)),
        # Translation: move by (tx, ty).
        ("translation", translation_matrix(300, -80), (150, 500), (1.0, 0.4, 0.4)),
        # Rotation: rotate 45 degrees about its own placed center.
        ("rotation", rotation_matrix(45, cx=150, cy=280), (150, 280), (0.4, 0.85, 1.0)),
        # Scaling: scale by (1.5, 0.7) about its own placed center.
        ("scaling", scaling_matrix(1.5, 0.7, cx=450, cy=280), (450, 280), (0.5, 1.0, 0.5)),
        # Shearing: shear in x, then place at a screen position via translation.
        ("shearing", matmul(translation_matrix(450, 60), shearing_matrix(0.6, 0.0)),
         (0, 0), (1.0, 0.85, 0.3)),
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

    # Pre-place the "original" shape at its screen center via a translation,
    # since base_shape is defined around the local origin.
    place_original = translation_matrix(*center)

    while not glfw.window_should_close(window):
        if glfw.get_key(window, glfw.KEY_ESCAPE) == glfw.PRESS:
            glfw.set_window_should_close(window, True)

        glClearColor(0.05, 0.05, 0.08, 1.0)
        glClear(GL_COLOR_BUFFER_BIT)

        glUseProgram(prog)
        glUniform2f(u_res, float(W), float(H))
        glBindVertexArray(vao)

        for label, matrix, pivot, color in scenes:
            if label == "original":
                pts = apply_transform(place_original, base_shape)
            elif label == "translation":
                placed = apply_transform(place_original, base_shape)
                pts = apply_transform(matrix, placed)
            elif label == "shearing":
                pts = apply_transform(matrix, base_shape)
            else:
                placed = apply_transform(translation_matrix(*pivot), base_shape)
                pts = apply_transform(matrix, placed)

            pts.append(pts[0])  # close the triangle outline
            upload(pts)
            glUniform3f(u_color, *color)
            glDrawArrays(GL_LINE_STRIP, 0, len(pts))

        glBindVertexArray(0)
        glfw.swap_buffers(window)
        glfw.poll_events()

    glfw.terminate()


if __name__ == "__main__":
    main()
