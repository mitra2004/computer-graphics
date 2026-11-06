// Experiment 1: dda bresenham
#include "common.h"

void dda(int x0,int y0,int x1,int y1) { int n=std::max(abs(x1-x0),abs(y1-y0)); float x=x0,y=y0,dx=(x1-x0)/(float)n,dy=(y1-y0)/(float)n; glBegin(GL_POINTS); for(int i=0;i<=n;i++,x+=dx,y+=dy) glVertex2i(lround(x),lround(y)); glEnd(); }
void bresenham(int x0,int y0,int x1,int y1) { int dx=abs(x1-x0), sx=x0<x1?1:-1, dy=-abs(y1-y0), sy=y0<y1?1:-1, e=dx+dy; glBegin(GL_POINTS); for(;;) { glVertex2i(x0,y0); if(x0==x1&&y0==y1) break; int e2=2*e; if(e2>=dy){e+=dy;x0+=sx;} if(e2<=dx){e+=dx;y0+=sy;} } glEnd(); }
void draw1(){glPointSize(5);glColor3f(.2,.85,1);dda(40,90,360,480);glColor3f(1,.65,.2);bresenham(440,90,760,480);}
