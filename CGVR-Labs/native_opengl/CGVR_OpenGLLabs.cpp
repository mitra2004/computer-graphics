// Native macOS OpenGL/GLUT implementations for experiments 1–8 and 10.
// Build with: clang++ -std=c++17 -DGL_SILENCE_DEPRECATION CGVR_OpenGLLabs.cpp -framework OpenGL -framework GLUT -o cgvr_opengl
// Run with:   ./cgvr_opengl <1-8|10>
#include <OpenGL/gl.h>
#include <OpenGL/glext.h>
#include <GLUT/glut.h>
#include <algorithm>
#include <array>
#include <cmath>
#include <cstring>
#include <cstdlib>
#include <iostream>
#include <vector>

int experiment = 1;
bool autoRun = false;
std::size_t autoIndex = 0;
const std::array<int, 9> autoSequence = {1, 2, 3, 4, 5, 6, 7, 8, 10};

void dda(int x0,int y0,int x1,int y1) { int n=std::max(abs(x1-x0),abs(y1-y0)); float x=x0,y=y0,dx=(x1-x0)/(float)n,dy=(y1-y0)/(float)n; glBegin(GL_POINTS); for(int i=0;i<=n;i++,x+=dx,y+=dy) glVertex2i(lround(x),lround(y)); glEnd(); }
void bresenham(int x0,int y0,int x1,int y1) { int dx=abs(x1-x0), sx=x0<x1?1:-1, dy=-abs(y1-y0), sy=y0<y1?1:-1, e=dx+dy; glBegin(GL_POINTS); for(;;) { glVertex2i(x0,y0); if(x0==x1&&y0==y1) break; int e2=2*e; if(e2>=dy){e+=dy;x0+=sx;} if(e2<=dx){e+=dx;y0+=sy;} } glEnd(); }
void midpointCircle(int cx,int cy,int r) { int x=0,y=r,d=1-r; glBegin(GL_POINTS); while(x<=y){ int p[][2]={{x,y},{y,x},{-x,y},{-y,x},{x,-y},{y,-x},{-x,-y},{-y,-x}}; for(auto &q:p) glVertex2i(cx+q[0],cy+q[1]); ++x; if(d<0)d+=2*x+1;else{--y;d+=2*(x-y)+1;} } glEnd(); }
struct P { float x,y,z; };
P transform(P p,const std::array<float,9>&m) { return {m[0]*p.x+m[1]*p.y+m[2],m[3]*p.x+m[4]*p.y+m[5],m[6]*p.x+m[7]*p.y+m[8]}; }
std::array<float,9> mul(const std::array<float,9>&a,const std::array<float,9>&b){std::array<float,9>r{};for(int i=0;i<3;i++)for(int j=0;j<3;j++)for(int k=0;k<3;k++)r[i*3+j]+=a[i*3+k]*b[k*3+j];return r;}
void polygon(const std::vector<P>&q) { glBegin(GL_LINE_LOOP);for(auto p:q)glVertex2f(p.x,p.y);glEnd(); }

int outcode(float x,float y) { return (x<180?1:0)|(x>620?2:0)|(y<120?4:0)|(y>480?8:0); }
bool cohen(P&a,P&b) { for(;;){int ca=outcode(a.x,a.y),cb=outcode(b.x,b.y);if(!(ca|cb))return true;if(ca&cb)return false;int c=ca?ca:cb;P q{};if(c&4){q.x=a.x+(b.x-a.x)*(120-a.y)/(b.y-a.y);q.y=120;}else if(c&8){q.x=a.x+(b.x-a.x)*(480-a.y)/(b.y-a.y);q.y=480;}else if(c&2){q.y=a.y+(b.y-a.y)*(620-a.x)/(b.x-a.x);q.x=620;}else{q.y=a.y+(b.y-a.y)*(180-a.x)/(b.x-a.x);q.x=180;}if(c==ca)a=q;else b=q;} }

GLuint shaderProgram=0;
GLuint compileShader(GLenum t,const char*s){GLuint h=glCreateShader(t);glShaderSource(h,1,&s,nullptr);glCompileShader(h);return h;}
void initShaders(){const char*vs="#version 120\nattribute vec2 p; attribute vec3 c; varying vec3 v; void main(){gl_Position=vec4(p,0.,1.);v=c;}";const char*fs="#version 120\nvarying vec3 v; void main(){gl_FragColor=vec4(v,1.);}";GLuint a=compileShader(GL_VERTEX_SHADER,vs),b=compileShader(GL_FRAGMENT_SHADER,fs);shaderProgram=glCreateProgram();glAttachShader(shaderProgram,a);glAttachShader(shaderProgram,b);glBindAttribLocation(shaderProgram,0,"p");glBindAttribLocation(shaderProgram,1,"c");glLinkProgram(shaderProgram);}

void draw1(){glPointSize(5);glColor3f(.2,.85,1);dda(40,90,360,480);glColor3f(1,.65,.2);bresenham(440,90,760,480);}
void draw2(){glPointSize(4);glColor3f(.3,1,.5);midpointCircle(400,300,190);}
void draw3(){std::vector<P>q={{-90,-60,1},{100,-55,1},{75,70,1},{-85,90,1}};glPushMatrix();glTranslatef(400,300,0);glColor3f(.4,.45,.55);polygon(q);glColor3f(.2,.85,1);glTranslatef(90,35,0);glRotatef(32,0,0,1);glScalef(1.25,.75,1);polygon(q);glPopMatrix();}
void draw4(){std::vector<P>q={{-65,-45,1},{65,-45,1},{65,45,1},{-65,45,1}};std::array<float,9>R={.707f,-.707f,0,.707f,.707f,0,0,0,1},T={1,0,160,0,1,40,0,0,1};auto M=mul(T,R);glPushMatrix();glTranslatef(400,300,0);glColor3f(.4,.45,.55);polygon(q);std::vector<P>r;for(auto p:q)r.push_back(transform(p,M));glColor3f(1,.75,.2);polygon(r);glPopMatrix();}
void draw5(){glColor3f(.8,.9,1);glBegin(GL_LINE_LOOP);glVertex2i(180,120);glVertex2i(620,120);glVertex2i(620,480);glVertex2i(180,480);glEnd();P a={50,170,0},b={750,430,0},u=a,v=b;glColor3f(1,.2,.25);glBegin(GL_LINES);glVertex2f(a.x,a.y);glVertex2f(b.x,b.y);glEnd();if(cohen(u,v)){glLineWidth(5);glColor3f(.2,1,.45);glBegin(GL_LINES);glVertex2f(u.x,u.y);glVertex2f(v.x,v.y);glEnd();glLineWidth(1);}}
void draw6(){glEnable(GL_DEPTH_TEST);glBegin(GL_TRIANGLES);glColor3f(.15,.55,1);glVertex3f(-.8,-.65,.3);glVertex3f(.85,-.55,-.35);glVertex3f(0,.85,0);glColor3f(1,.4,.1);glVertex3f(-.7,.65,-.45);glVertex3f(.9,.45,.5);glVertex3f(0,-.85,-.2);glEnd();glDisable(GL_DEPTH_TEST);}
void draw7(){glEnable(GL_DEPTH_TEST);glEnable(GL_LIGHTING);glEnable(GL_LIGHT0);GLfloat pos[]={-2,3,4,1},amb[]={.1f,.1f,.1f,1},dif[]={.2f,.65f,1,1},spec[]={1,1,1,1};glLightfv(GL_LIGHT0,GL_POSITION,pos);glLightfv(GL_LIGHT0,GL_AMBIENT,amb);glLightfv(GL_LIGHT0,GL_DIFFUSE,dif);glMaterialfv(GL_FRONT,GL_SPECULAR,spec);glMaterialf(GL_FRONT,GL_SHININESS,56);glutSolidSphere(1.4,64,48);glDisable(GL_LIGHTING);glDisable(GL_DEPTH_TEST);}
void draw8(){if(!shaderProgram)initShaders();glUseProgram(shaderProgram);glBegin(GL_TRIANGLES);glVertexAttrib3f(1,1,0,0);glVertexAttrib2f(0,0,.75);glVertexAttrib3f(1,0,1,0);glVertexAttrib2f(0,-.7,-.65);glVertexAttrib3f(1,0,0,1);glVertexAttrib2f(0,.7,-.65);glEnd();glUseProgram(0);}
void draw10(){glEnable(GL_DEPTH_TEST);glMatrixMode(GL_PROJECTION);glLoadIdentity();gluPerspective(55,800.0/600,1,20);glMatrixMode(GL_MODELVIEW);glLoadIdentity();gluLookAt(3,2.5,6,0,0,0,0,1,0);glRotatef(30,1,1,0);glColor3f(.25,.75,1);glutWireCube(2.4);glDisable(GL_DEPTH_TEST);}
void display(){glClearColor(.025,.04,.075,1);glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);glMatrixMode(GL_PROJECTION);glLoadIdentity();if(experiment==6||experiment==7){gluPerspective(45,800.0/600,1,20);glMatrixMode(GL_MODELVIEW);glLoadIdentity();gluLookAt(0,0,6,0,0,0,0,1,0);}else if(experiment!=10){glOrtho(0,800,0,600,-10,10);glMatrixMode(GL_MODELVIEW);glLoadIdentity();}switch(experiment){case 1:draw1();break;case 2:draw2();break;case 3:draw3();break;case 4:draw4();break;case 5:draw5();break;case 6:draw6();break;case 7:draw7();break;case 8:draw8();break;case 10:draw10();}glutSwapBuffers();}
void nextExperiment(int) {
    autoIndex = (autoIndex + 1) % autoSequence.size();
    experiment = autoSequence[autoIndex];
    std::string title = "CGVR Auto Run — Experiment " + std::to_string(experiment) + " (6 seconds)";
    glutSetWindowTitle(title.c_str());
    glutPostRedisplay();
    glutTimerFunc(6000, nextExperiment, 0);
}
int main(int argc,char**argv){
    if(argc>1 && std::strcmp(argv[1], "all")==0) { autoRun=true; experiment=autoSequence[0]; }
    else if(argc>1) experiment=atoi(argv[1]);
    if(experiment<1||(experiment>8&&experiment!=10)){std::cerr<<"Use experiment 1–8, 10, or all\n";return 1;}
    glutInit(&argc,argv);glutInitDisplayMode(GLUT_DOUBLE|GLUT_RGB|GLUT_DEPTH);glutInitWindowSize(800,600);
    std::string title = autoRun ? "CGVR Auto Run — Experiment 1 (6 seconds)" : "CGVR Experiment "+std::to_string(experiment);
    glutCreateWindow(title.c_str());glutDisplayFunc(display);
    if(autoRun) glutTimerFunc(6000, nextExperiment, 0);
    glutMainLoop();
}
