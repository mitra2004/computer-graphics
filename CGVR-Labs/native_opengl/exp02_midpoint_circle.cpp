// Experiment 2: midpoint circle
#include "common.h"

void midpointCircle(int cx,int cy,int r) { int x=0,y=r,d=1-r; glBegin(GL_POINTS); while(x<=y){ int p[][2]={{x,y},{y,x},{-x,y},{-y,x},{x,-y},{y,-x},{-x,-y},{-y,-x}}; for(auto &q:p) glVertex2i(cx+q[0],cy+q[1]); ++x; if(d<0)d+=2*x+1;else{--y;d+=2*(x-y)+1;} } glEnd(); }
void draw2(){glPointSize(4);glColor3f(.3,1,.5);midpointCircle(400,300,190);}
