#include "common.h"

void polygon(const std::vector<P>&q) { glBegin(GL_LINE_LOOP);for(auto p:q)glVertex2f(p.x,p.y);glEnd(); }
