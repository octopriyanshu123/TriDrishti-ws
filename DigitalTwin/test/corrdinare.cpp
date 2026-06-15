#include <GL/glut.h>
#include <cmath>
#include <cstdlib>

// Camera variables
float cameraX = 0.0f, cameraY = 15.0f, cameraZ = 15.0f;
float lookX = 0.0f, lookY = 0.0f, lookZ = 0.0f;


float angleX = 0.0f, angleY = 0.0f;
float mouseX = 0.0f, mouseY = 0.0f;
bool mousePressed = false;

// Window dimensions
int windowWidth = 800;
int windowHeight = 600;

// Camera movement speed
float moveSpeed = 0.5f;

// Update camera position based on angles
void updateCamera() {
    float radius = sqrt(cameraX*cameraX + cameraY*cameraY + cameraZ*cameraZ);
    cameraX = radius * sin(angleX * 3.14159f / 180.0f) * cos(angleY * 3.14159f / 180.0f);
    cameraY = radius * sin(angleY * 3.14159f / 180.0f);
    cameraZ = radius * cos(angleX * 3.14159f / 180.0f) * cos(angleY * 3.14159f / 180.0f);
    
    lookX = 0.0f;
    lookY = 0.0f;
    lookZ = 0.0f;
}

// Draw grid
void drawGrid() {
    glColor3f(0.5f, 0.5f, 0.5f);
    glBegin(GL_LINES);
    
    // Draw grid lines
    for(float i = -10.0f; i <= 10.0f; i += 1.0f) {
        // Lines parallel to X axis
        glVertex3f(-10.0f, 0.0f, i);
        glVertex3f(10.0f, 0.0f, i);
        
        // Lines parallel to Z axis
        glVertex3f(i, 0.0f, -10.0f);
        glVertex3f(i, 0.0f, 10.0f);
    }
    
    glEnd();
    
    // Draw X axis line (red)
    glColor3f(1.0f, 0.0f, 0.0f);
    glBegin(GL_LINES);
    glVertex3f(-10.0f, 0.0f, 0.0f);
    glVertex3f(10.0f, 0.0f, 0.0f);
    glEnd();
    
    // Draw Z axis line (blue)
    glColor3f(0.0f, 0.0f, 1.0f);
    glBegin(GL_LINES);
    glVertex3f(0.0f, 0.0f, -10.0f);
    glVertex3f(0.0f, 0.0f, 10.0f);
    glEnd();
}

// Draw axes at origin (0,0,0)
void drawAxes() {
    glLineWidth(3.0f);
    
    // X axis (red)
    glColor3f(1.0f, 0.0f, 0.0f);
    glBegin(GL_LINES);
    glVertex3f(0.0f, 0.0f, 0.0f);
    glVertex3f(3.0f, 0.0f, 0.0f);
    glEnd();
    
    // Y axis (green)
    glColor3f(0.0f, 1.0f, 0.0f);
    glBegin(GL_LINES);
    glVertex3f(0.0f, 0.0f, 0.0f);
    glVertex3f(0.0f, 3.0f, 0.0f);
    glEnd();
    
    // Z axis (blue)
    glColor3f(0.0f, 0.0f, 1.0f);
    glBegin(GL_LINES);
    glVertex3f(0.0f, 0.0f, 0.0f);
    glVertex3f(0.0f, 0.0f, 3.0f);
    glEnd();
    
    // Draw cones at the end of axes
    // X axis cone
    glColor3f(1.0f, 0.0f, 0.0f);
    glPushMatrix();
    glTranslatef(3.0f, 0.0f, 0.0f);
    glutSolidCone(0.2f, 0.5f, 10, 10);
    glPopMatrix();
    
    // Y axis cone
    glColor3f(0.0f, 1.0f, 0.0f);
    glPushMatrix();
    glTranslatef(0.0f, 3.0f, 0.0f);
    glutSolidCone(0.2f, 0.5f, 10, 10);
    glPopMatrix();
    
    // Z axis cone
    glColor3f(0.0f, 0.0f, 1.0f);
    glPushMatrix();
    glTranslatef(0.0f, 0.0f, 3.0f);
    glutSolidCone(0.2f, 0.5f, 10, 10);
    glPopMatrix();
    
    glLineWidth(1.0f);
}

// Draw a small sphere at origin
void drawOriginMarker() {
    glColor3f(1.0f, 1.0f, 0.0f);
    glPushMatrix();
    glTranslatef(0.0f, 0.0f, 0.0f);
    glutSolidSphere(0.1f, 20, 20);
    glPopMatrix();
}

// Display callback
void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    
    // Set camera position
    gluLookAt(cameraX, cameraY, cameraZ, lookX, lookY, lookZ, 0.0f, 1.0f, 0.0f);
    
    // Draw grid
    drawGrid();
    
    // Draw axes
    drawAxes();
    
    // Draw origin marker
    drawOriginMarker();
    
    glutSwapBuffers();
}

// Reshape callback
void reshape(int w, int h) {
    windowWidth = w;
    windowHeight = h;
    
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0f, (float)w / (float)h, 0.1f, 100.0f);
    glMatrixMode(GL_MODELVIEW);
}

// Mouse button callback
void mouseButton(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON) {
        if (state == GLUT_DOWN) {
            mousePressed = true;
            mouseX = x;
            mouseY = y;
        } else {
            mousePressed = false;
        }
    }
}

// Mouse motion callback
void mouseMotion(int x, int y) {
    if (mousePressed) {
        // Calculate mouse movement
        float dx = x - mouseX;
        float dy = y - mouseY;
        
        // Update camera angles
        angleX -= dx * 0.5f;
        angleY += dy * 0.5f;
        
        // Limit vertical angle to avoid gimbal lock
        if (angleY > 89.0f) angleY = 89.0f;
        if (angleY < -89.0f) angleY = -89.0f;
        
        // Update camera position
        updateCamera();
        
        mouseX = x;
        mouseY = y;
        
        glutPostRedisplay();
    }
}

// Keyboard callback
void keyboard(unsigned char key, int x, int y) {
    float radius = sqrt(cameraX*cameraX + cameraY*cameraY + cameraZ*cameraZ);
    
    switch(key) {
        case 'w': // Move forward
        case 'W':
            radius -= moveSpeed;
            if (radius < 1.0f) radius = 1.0f;
            cameraX = radius * sin(angleX * 3.14159f / 180.0f) * cos(angleY * 3.14159f / 180.0f);
            cameraY = radius * sin(angleY * 3.14159f / 180.0f);
            cameraZ = radius * cos(angleX * 3.14159f / 180.0f) * cos(angleY * 3.14159f / 180.0f);
            break;
            
        case 's': // Move backward
        case 'S':
            radius += moveSpeed;
            cameraX = radius * sin(angleX * 3.14159f / 180.0f) * cos(angleY * 3.14159f / 180.0f);
            cameraY = radius * sin(angleY * 3.14159f / 180.0f);
            cameraZ = radius * cos(angleX * 3.14159f / 180.0f) * cos(angleY * 3.14159f / 180.0f);
            break;
            
        case 'a': // Rotate left
        case 'A':
            angleX -= 5.0f;
            updateCamera();
            break;
            
        case 'd': // Rotate right
        case 'D':
            angleX += 5.0f;
            updateCamera();
            break;
            
        case 'q': // Move up
        case 'Q':
            cameraY += moveSpeed;
            break;
            
        case 'e': // Move down
        case 'E':
            cameraY -= moveSpeed;
            break;
            
        case 27: // ESC key
            exit(0);
            break;
    }
    
    glutPostRedisplay();
}

// Special keyboard callback for arrow keys
void specialKeys(int key, int x, int y) {
    switch(key) {
        case GLUT_KEY_UP:
            cameraY += moveSpeed;
            break;
        case GLUT_KEY_DOWN:
            cameraY -= moveSpeed;
            break;
        case GLUT_KEY_LEFT:
            angleX -= 5.0f;
            updateCamera();
            break;
        case GLUT_KEY_RIGHT:
            angleX += 5.0f;
            updateCamera();
            break;
    }
    glutPostRedisplay();
}

// Initialize OpenGL settings
void initOpenGL() {
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    
    // Set up lighting
    GLfloat lightPos[] = {5.0f, 10.0f, 5.0f, 1.0f};
    GLfloat lightAmbient[] = {0.2f, 0.2f, 0.2f, 1.0f};
    GLfloat lightDiffuse[] = {0.8f, 0.8f, 0.8f, 1.0f};
    
    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
    glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse);
    
    // Enable color material
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
    
    // Initialize camera
    updateCamera();
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(windowWidth, windowHeight);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("3D Space with Camera - GLUT");
    
    initOpenGL();
    
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutMouseFunc(mouseButton);
    glutMotionFunc(mouseMotion);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(specialKeys);
    
    glutMainLoop();
    
    return 0;
}