#include <GL/glut.h>

GLUquadric* quad = NULL;

void drawTank() {
    // Tank body (cylinder)
    glColor3f(0.5f, 0.5f, 0.6f);  // Steel color
    gluCylinder(quad, 2.0f, 2.0f, 4.0f, 32, 32);
    

}

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    
    // Camera position
    gluLookAt(8, 6, 12, 0, 2, 0, 0, 1, 0);
    
    // Rotate cylinder to make Z-axis point up
    glPushMatrix();
    glRotatef(-90, 1, 0, 0);
    drawTank();
    glPopMatrix();
    
    glutSwapBuffers();
}

void reshape(int w, int h) {
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45, (float)w/h, 0.1, 100);
    glMatrixMode(GL_MODELVIEW);
}

void init() {
    glClearColor(0.2f, 0.2f, 0.3f, 1);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    
    // Lighting
    GLfloat lightPos[] = {5, 10, 5, 1};
    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
    glEnable(GL_COLOR_MATERIAL);
    
    // Create quadric
    quad = gluNewQuadric();
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(800, 600);
    glutCreateWindow("Oil Tank");
    
    init();
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    
    glutMainLoop();
    
    gluDeleteQuadric(quad);
    return 0;
}