// Experiment 3: 2d transformations
#include "common.h"

void draw3(){std::vector<P>q={{-90,-60,1},{100,-55,1},{75,70,1},{-85,90,1}};glPushMatrix();glTranslatef(400,300,0);glColor3f(.4,.45,.55);polygon(q);glColor3f(.2,.85,1);glTranslatef(90,35,0);glRotatef(32,0,0,1);glScalef(1.25,.75,1);polygon(q);glPopMatrix();}
