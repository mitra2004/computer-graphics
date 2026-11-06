// Experiment 7: phong lighting
#include "common.h"

void draw7(){glEnable(GL_DEPTH_TEST);glEnable(GL_LIGHTING);glEnable(GL_LIGHT0);GLfloat pos[]={-2,3,4,1},amb[]={.1f,.1f,.1f,1},dif[]={.2f,.65f,1,1},spec[]={1,1,1,1};glLightfv(GL_LIGHT0,GL_POSITION,pos);glLightfv(GL_LIGHT0,GL_AMBIENT,amb);glLightfv(GL_LIGHT0,GL_DIFFUSE,dif);glMaterialfv(GL_FRONT,GL_SPECULAR,spec);glMaterialf(GL_FRONT,GL_SHININESS,56);glutSolidSphere(1.4,64,48);glDisable(GL_LIGHTING);glDisable(GL_DEPTH_TEST);}
