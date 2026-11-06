// Experiment 6: depth buffer
#include "common.h"

void draw6(){glEnable(GL_DEPTH_TEST);glBegin(GL_TRIANGLES);glColor3f(.15,.55,1);glVertex3f(-.8,-.65,.3);glVertex3f(.85,-.55,-.35);glVertex3f(0,.85,0);glColor3f(1,.4,.1);glVertex3f(-.7,.65,-.45);glVertex3f(.9,.45,.5);glVertex3f(0,-.85,-.2);glEnd();glDisable(GL_DEPTH_TEST);}
