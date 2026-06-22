#include <GL/glut.h>

#include "core/panel.cpp"

 

void display()
{
    glClear(GL_COLOR_BUFFER_BIT);

    panel.draw();

    glutSwapBuffers();
}

int main(int argc,char** argv)
{
    glutInit(&argc,argv);

    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(800,600);

    glutCreateWindow("GUI");

    gluOrtho2D(0,800,0,600);

    glutDisplayFunc(display);

    glutMainLoop();
}