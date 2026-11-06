CXX := clang++
CXXFLAGS := -std=c++17 -Wall -Wextra -DGL_SILENCE_DEPRECATION
FRAMEWORKS := -framework OpenGL -framework GLUT

SRCS := $(wildcard *.cpp)
cgvr_opengl: $(SRCS) common.h
	$(CXX) $(CXXFLAGS) $(SRCS) $(FRAMEWORKS) -o $@

run-%: cgvr_opengl
	./cgvr_opengl $*

run-all: cgvr_opengl
	./cgvr_opengl all

clean:
	rm -f cgvr_opengl
