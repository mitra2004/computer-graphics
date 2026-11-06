// Experiment 4: matrix transformations
#include "common.h"

P transform(P p,const std::array<float,9>&m) { return {m[0]*p.x+m[1]*p.y+m[2],m[3]*p.x+m[4]*p.y+m[5],m[6]*p.x+m[7]*p.y+m[8]}; }
std::array<float,9> mul(const std::array<float,9>&a,const std::array<float,9>&b){std::array<float,9>r{};for(int i=0;i<3;i++)for(int j=0;j<3;j++)for(int k=0;k<3;k++)r[i*3+j]+=a[i*3+k]*b[k*3+j];return r;}
void draw4(){std::vector<P>q={{-65,-45,1},{65,-45,1},{65,45,1},{-65,45,1}};std::array<float,9>R={.707f,-.707f,0,.707f,.707f,0,0,0,1},T={1,0,160,0,1,40,0,0,1};auto M=mul(T,R);glPushMatrix();glTranslatef(400,300,0);glColor3f(.4,.45,.55);polygon(q);std::vector<P>r;for(auto p:q)r.push_back(transform(p,M));glColor3f(1,.75,.2);polygon(r);glPopMatrix();}
