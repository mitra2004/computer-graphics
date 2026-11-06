// Experiment 10: perspective viewing
#include "common.h"

void draw10(){glEnable(GL_DEPTH_TEST);glMatrixMode(GL_PROJECTION);glLoadIdentity();gluPerspective(55,800.0/600,1,20);glMatrixMode(GL_MODELVIEW);glLoadIdentity();gluLookAt(3,2.5,6,0,0,0,0,1,0);glRotatef(30,1,1,0);glColor3f(.25,.75,1);glutWireCube(2.4);glDisable(GL_DEPTH_TEST);}
