// Native macOS OpenGL/GLUT driver for experiments 1-8 and 10.
// Build with `make`; run with ./cgvr_opengl <1-8|10|all>
#include "common.h"

int experiment = 1;
bool autoRun = false;
std::size_t autoIndex = 0;
const std::array<int, 9> autoSequence = {1, 2, 3, 4, 5, 6, 7, 8, 10};
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
