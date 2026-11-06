// Experiment 5: cohen sutherland clipping
#include "common.h"

int outcode(float x,float y) { return (x<180?1:0)|(x>620?2:0)|(y<120?4:0)|(y>480?8:0); }
bool cohen(P&a,P&b) { for(;;){int ca=outcode(a.x,a.y),cb=outcode(b.x,b.y);if(!(ca|cb))return true;if(ca&cb)return false;int c=ca?ca:cb;P q{};if(c&4){q.x=a.x+(b.x-a.x)*(120-a.y)/(b.y-a.y);q.y=120;}else if(c&8){q.x=a.x+(b.x-a.x)*(480-a.y)/(b.y-a.y);q.y=480;}else if(c&2){q.y=a.y+(b.y-a.y)*(620-a.x)/(b.x-a.x);q.x=620;}else{q.y=a.y+(b.y-a.y)*(180-a.x)/(b.x-a.x);q.x=180;}if(c==ca)a=q;else b=q;} }
void draw5(){glColor3f(.8,.9,1);glBegin(GL_LINE_LOOP);glVertex2i(180,120);glVertex2i(620,120);glVertex2i(620,480);glVertex2i(180,480);glEnd();P a={50,170,0},b={750,430,0},u=a,v=b;glColor3f(1,.2,.25);glBegin(GL_LINES);glVertex2f(a.x,a.y);glVertex2f(b.x,b.y);glEnd();if(cohen(u,v)){glLineWidth(5);glColor3f(.2,1,.45);glBegin(GL_LINES);glVertex2f(u.x,u.y);glVertex2f(v.x,v.y);glEnd();glLineWidth(1);}}
