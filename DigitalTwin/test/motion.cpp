#include <GL/glut.h>
#include <cmath>
#include <cstdlib>
#include <iostream>

// Fixed camera position at (0,15,15)
const float CAMERA_X = 0.0f;
const float CAMERA_Y = 15.0f;
const float CAMERA_Z = 15.0f;

// Robot ball variables
float ballX = 0.0f, ballY = 0.0f, ballZ = 0.0f;  // Position
float ballRotX = 0.0f, ballRotY = 0.0f, ballRotZ = 0.0f;  // Orientation (degrees)
float ballRadius = 1.0f;

// Mouse interaction for ball rotation
float lastMouseX = 0.0f, lastMouseY = 0.0f;
bool mousePressed = false;
int selectedAxis = -1;  // 0=X, 1=Y, 2=Z for orientation, 3=translate
bool rotationMode = true;  // True for rotation, false for translation

// Translation speed
float translateSpeed = 0.2f;

// Draw sphere with local axes
void drawSphereWithAxes() {
    // Draw the sphere (semi-transparent)
    glColor4f(0.6f, 0.6f, 0.8f, 0.7f);
    glutSolidSphere(ballRadius, 50, 50);
    
    // Draw outline for better visibility
    glColor3f(0.3f, 0.3f, 0.5f);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glutSolidSphere(ballRadius, 50, 50);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    
    // Draw local axes on the sphere
    glLineWidth(3.0f);
    
    // X axis (red)
    glColor3f(1.0f, 0.0f, 0.0f);
    glBegin(GL_LINES);
    glVertex3f(0.0f, 0.0f, 0.0f);
    glVertex3f(ballRadius + 0.3f, 0.0f, 0.0f);
    glEnd();
    
    // Arrow head for X
    glPushMatrix();
    glTranslatef(ballRadius + 0.3f, 0.0f, 0.0f);
    glColor3f(1.0f, 0.0f, 0.0f);
    glutSolidCone(0.08f, 0.2f, 10, 10);
    glPopMatrix();
    
    // Y axis (green)
    glColor3f(0.0f, 1.0f, 0.0f);
    glBegin(GL_LINES);
    glVertex3f(0.0f, 0.0f, 0.0f);
    glVertex3f(0.0f, ballRadius + 0.3f, 0.0f);
    glEnd();
    
    // Arrow head for Y
    glPushMatrix();
    glTranslatef(0.0f, ballRadius + 0.3f, 0.0f);
    glColor3f(0.0f, 1.0f, 0.0f);
    glutSolidCone(0.08f, 0.2f, 10, 10);
    glPopMatrix();
    
    // Z axis (blue)
    glColor3f(0.0f, 0.0f, 1.0f);
    glBegin(GL_LINES);
    glVertex3f(0.0f, 0.0f, 0.0f);
    glVertex3f(0.0f, 0.0f, ballRadius + 0.3f);
    glEnd();
    
    // Arrow head for Z
    glPushMatrix();
    glTranslatef(0.0f, 0.0f, ballRadius + 0.3f);
    glColor3f(0.0f, 0.0f, 1.0f);
    glutSolidCone(0.08f, 0.2f, 10, 10);
    glPopMatrix();
    
    glLineWidth(1.0f);
    
    // Add eye-like features for better orientation
    glColor3f(0.0f, 0.0f, 0.0f);
    glPushMatrix();
    glTranslatef(ballRadius * 0.6f, ballRadius * 0.6f, ballRadius * 0.8f);
    glutSolidSphere(0.1f, 20, 20);
    glPopMatrix();
    
    glPushMatrix();
    glTranslatef(-ballRadius * 0.6f, ballRadius * 0.6f, ballRadius * 0.8f);
    glutSolidSphere(0.1f, 20, 20);
    glPopMatrix();
    
    // Add a nose to show front
    glColor3f(1.0f, 0.5f, 0.0f);
    glPushMatrix();
    glTranslatef(0.0f, 0.0f, ballRadius + 0.1f);
    glutSolidSphere(0.08f, 20, 20);
    glPopMatrix();
}

// Draw world grid
void drawGrid() {
    glColor3f(0.5f, 0.5f, 0.5f);
    glBegin(GL_LINES);
    
    // Draw grid lines from -10 to 10
    for(float i = -10.0f; i <= 10.0f; i += 1.0f) {
        // Lines parallel to X axis
        glVertex3f(-10.0f, -ballRadius, i);
        glVertex3f(10.0f, -ballRadius, i);
        
        // Lines parallel to Z axis
        glVertex3f(i, -ballRadius, -10.0f);
        glVertex3f(i, -ballRadius, 10.0f);
    }
    
    glEnd();
    
    // Draw X axis line (red)
    glColor3f(1.0f, 0.0f, 0.0f);
    glBegin(GL_LINES);
    glVertex3f(-12.0f, -ballRadius, 0.0f);
    glVertex3f(12.0f, -ballRadius, 0.0f);
    glEnd();
    
    // Draw Z axis line (blue)
    glColor3f(0.0f, 0.0f, 1.0f);
    glBegin(GL_LINES);
    glVertex3f(0.0f, -ballRadius, -12.0f);
    glVertex3f(0.0f, -ballRadius, 12.0f);
    glEnd();
}
void drawText(float x, float y, const char* text) {
    glDisable(GL_LIGHTING);
    glColor3f(1.0f, 1.0f, 1.0f);
    glRasterPos2f(x, y);
    for (const char* c = text; *c != '\0'; c++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
    }
    glEnable(GL_LIGHTING);
}

// Draw world axes at origin (0,0,0)
void drawWorldAxes() {
    glLineWidth(3.0f);
    
    // X axis (red)
    glColor3f(1.0f, 0.0f, 0.0f);
    glBegin(GL_LINES);
    glVertex3f(0.0f, -ballRadius, 0.0f);
    glVertex3f(3.0f, -ballRadius, 0.0f);
    glEnd();
    
    // Arrow for X
    glPushMatrix();
    glTranslatef(3.0f, -ballRadius, 0.0f);
    glColor3f(1.0f, 0.0f, 0.0f);
    glutSolidCone(0.1f, 0.3f, 10, 10);
    glPopMatrix();
    
    // Y axis (green)
    glColor3f(0.0f, 1.0f, 0.0f);
    glBegin(GL_LINES);
    glVertex3f(0.0f, -ballRadius, 0.0f);
    glVertex3f(0.0f, 3.0f - ballRadius, 0.0f);
    glEnd();
    
    // Arrow for Y
    glPushMatrix();
    glTranslatef(0.0f, 3.0f - ballRadius, 0.0f);
    glColor3f(0.0f, 1.0f, 0.0f);
    glutSolidCone(0.1f, 0.3f, 10, 10);
    glPopMatrix();
    
    // Z axis (blue)
    glColor3f(0.0f, 0.0f, 1.0f);
    glBegin(GL_LINES);
    glVertex3f(0.0f, -ballRadius, 0.0f);
    glVertex3f(0.0f, -ballRadius, 3.0f);
    glEnd();
    
    // Arrow for Z
    glPushMatrix();
    glTranslatef(0.0f, -ballRadius, 3.0f);
    glColor3f(0.0f, 0.0f, 1.0f);
    glutSolidCone(0.1f, 0.3f, 10, 10);
    glPopMatrix();
    
    glLineWidth(1.0f);
    
    // Draw origin marker
    glColor3f(1.0f, 1.0f, 0.0f);
    glPushMatrix();
    glTranslatef(0.0f, -ballRadius, 0.0f);
    glutSolidSphere(0.08f, 20, 20);
    glPopMatrix();



    drawText(15,0,"x");
}

// Display text

// Display callback
void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    
    // Set fixed camera at (0,15,15)
    gluLookAt(CAMERA_X, CAMERA_Y, CAMERA_Z,  // Camera position
              0.0f, 0.0f, 0.0f,              // Look at origin
              0.0f, 1.0f, 0.0f);             // Up vector
    
    // Draw world grid and axes
    drawGrid();
    drawWorldAxes();
    
    // Draw ball with its local axes
    glPushMatrix();
    
    // IMPORTANT: Translate first, then rotate
    // This makes the ball rotate around its own center
    glTranslatef(ballX, ballY, ballZ);
    glRotatef(ballRotX, 1.0f, 0.0f, 0.0f);
    glRotatef(ballRotY, 0.0f, 1.0f, 0.0f);
    glRotatef(ballRotZ, 0.0f, 0.0f, 1.0f);
    
    drawSphereWithAxes();
    glPopMatrix();
    
    // Draw UI text (2D overlay)
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, glutGet(GLUT_WINDOW_WIDTH), 0, glutGet(GLUT_WINDOW_HEIGHT));
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    
    char info[256];
    sprintf(info, "Ball Position: (%.2f, %.2f, %.2f)", ballX, ballY, ballZ);
    drawText(10, glutGet(GLUT_WINDOW_HEIGHT) - 30, info);
    
    sprintf(info, "Ball Rotation: (%.0f, %.0f, %.0f)", ballRotX, ballRotY, ballRotZ);
    drawText(10, glutGet(GLUT_WINDOW_HEIGHT) - 55, info);
    
    if (rotationMode) {
        drawText(10, glutGet(GLUT_WINDOW_HEIGHT) - 85, "Mode: ROTATION");
        if (selectedAxis == 0) drawText(10, glutGet(GLUT_WINDOW_HEIGHT) - 105, "Selected: X-AXIS (Red)");
        else if (selectedAxis == 1) drawText(10, glutGet(GLUT_WINDOW_HEIGHT) - 105, "Selected: Y-AXIS (Green)");
        else if (selectedAxis == 2) drawText(10, glutGet(GLUT_WINDOW_HEIGHT) - 105, "Selected: Z-AXIS (Blue)");
        else drawText(10, glutGet(GLUT_WINDOW_HEIGHT) - 105, "Selected: None (Press X/Y/Z)");
    } else {
        drawText(10, glutGet(GLUT_WINDOW_HEIGHT) - 85, "Mode: TRANSLATION");
        drawText(10, glutGet(GLUT_WINDOW_HEIGHT) - 105, "Drag mouse to move ball");
    }
    
    drawText(10, glutGet(GLUT_WINDOW_HEIGHT) - 135, "Controls:");
    drawText(10, glutGet(GLUT_WINDOW_HEIGHT) - 155, "W/S: Move +Z/-Z");
    drawText(10, glutGet(GLUT_WINDOW_HEIGHT) - 175, "A/D: Move -X/+X");
    drawText(10, glutGet(GLUT_WINDOW_HEIGHT) - 195, "Q/E: Move +Y/-Y");
    drawText(10, glutGet(GLUT_WINDOW_HEIGHT) - 215, "X/Y/Z: Select rotation axis");
    drawText(10, glutGet(GLUT_WINDOW_HEIGHT) - 235, "Mouse drag: Rotate around selected axis");
    drawText(10, glutGet(GLUT_WINDOW_HEIGHT) - 255, "T: Toggle rotation/translation mode");
    drawText(10, glutGet(GLUT_WINDOW_HEIGHT) - 275, "R: Reset ball position and orientation");
    
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    
    glutSwapBuffers();
}

// Reshape callback
void reshape(int w, int h) {
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
            lastMouseX = x;
            lastMouseY = y;
        } else {
            mousePressed = false;
        }
    }
}

// Mouse motion callback for rotation/translation
void mouseMotion(int x, int y) {
    if (mousePressed) {
        float dx = (x - lastMouseX) * 0.5f;
        float dy = (y - lastMouseY) * 0.5f;
        
        if (rotationMode) {
            // Rotate around selected axis
            switch(selectedAxis) {
                case 0: // X axis
                    ballRotX += dy;
                    // Keep angles in 0-360 range
                    while (ballRotX >= 360.0f) ballRotX -= 360.0f;
                    while (ballRotX < 0.0f) ballRotX += 360.0f;
                    break;
                case 1: // Y axis
                    ballRotY += dx;
                    while (ballRotY >= 360.0f) ballRotY -= 360.0f;
                    while (ballRotY < 0.0f) ballRotY += 360.0f;
                    break;
                case 2: // Z axis
                    ballRotZ += dx;
                    while (ballRotZ >= 360.0f) ballRotZ -= 360.0f;
                    while (ballRotZ < 0.0f) ballRotZ += 360.0f;
                    break;
            }
        } else {
            // Translation mode - move ball in XZ plane
            ballX += dx * 0.05f;
            ballZ -= dy * 0.05f;
            
            // Clamp position to reasonable bounds
            if (ballX > 8.0f) ballX = 8.0f;
            if (ballX < -8.0f) ballX = -8.0f;
            if (ballZ > 8.0f) ballZ = 8.0f;
            if (ballZ < -8.0f) ballZ = -8.0f;
        }
        
        lastMouseX = x;
        lastMouseY = y;
        glutPostRedisplay();
    }
}

// Keyboard callback
void keyboard(unsigned char key, int x, int y) {
    switch(key) {
        // Translation controls
        case 'w':
        case 'W':
            ballZ += translateSpeed;
            if (ballZ > 9.0f) ballZ = 9.0f;
            break;
        case 's':
        case 'S':
            ballZ -= translateSpeed;
            if (ballZ < -9.0f) ballZ = -9.0f;
            break;
        case 'a':
        case 'A':
            ballX -= translateSpeed;
            if (ballX < -9.0f) ballX = -9.0f;
            break;
        case 'd':
        case 'D':
            ballX += translateSpeed;
            if (ballX > 9.0f) ballX = 9.0f;
            break;
        case 'q':
        case 'Q':
            ballY += translateSpeed;
            if (ballY > 5.0f) ballY = 5.0f;
            break;
        case 'e':
        case 'E':
            ballY -= translateSpeed;
            if (ballY < -1.0f) ballY = -1.0f;
            break;
        
        // Axis selection for rotation
        case 'x':
        case 'X':
            selectedAxis = 0;
            rotationMode = true;
            std::cout << "Selected X-axis rotation" << std::endl;
            break;
        case 'y':
        case 'Y':
            selectedAxis = 1;
            rotationMode = true;
            std::cout << "Selected Y-axis rotation" << std::endl;
            break;
        case 'z':
        case 'Z':
            selectedAxis = 2;
            rotationMode = true;
            std::cout << "Selected Z-axis rotation" << std::endl;
            break;
        
        // Toggle rotation/translation mode
        case 't':
        case 'T':
            rotationMode = !rotationMode;
            if (rotationMode) {
                std::cout << "Switched to ROTATION mode - Select axis (X/Y/Z) and drag mouse" << std::endl;
            } else {
                std::cout << "Switched to TRANSLATION mode - Drag mouse to move ball" << std::endl;
            }
            break;
        
        // Reset ball
        case 'r':
        case 'R':
            ballX = 0.0f;
            ballY = 0.0f;
            ballZ = 0.0f;
            ballRotX = 0.0f;
            ballRotY = 0.0f;
            ballRotZ = 0.0f;
            selectedAxis = -1;
            rotationMode = true;
            std::cout << "Ball reset to origin with zero rotation" << std::endl;
            break;
        
        case 27: // ESC
            exit(0);
            break;
    }
    
    glutPostRedisplay();
}

// Initialize OpenGL settings
void initOpenGL() {
    glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // Set up lighting
    GLfloat lightPos[] = {5.0f, 10.0f, 5.0f, 1.0f};
    GLfloat lightAmbient[] = {0.3f, 0.3f, 0.3f, 1.0f};
    GLfloat lightDiffuse[] = {0.8f, 0.8f, 0.8f, 1.0f};
    GLfloat lightSpecular[] = {1.0f, 1.0f, 1.0f, 1.0f};
    
    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
    glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpecular);
    
    // Enable color material
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
    
    // Set material properties
    glMaterialfv(GL_FRONT, GL_SPECULAR, lightSpecular);
    glMateriali(GL_FRONT, GL_SHININESS, 100);
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(1024, 768);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("3D Robotic Ball with Local Axes");
    
    initOpenGL();
    
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutMouseFunc(mouseButton);
    glutMotionFunc(mouseMotion);
    glutKeyboardFunc(keyboard);
    
    std::cout << "=== 3D Ball Control System ===" << std::endl;
    std::cout << "Camera fixed at (0,15,15)" << std::endl;
    std::cout << "Press X, Y, or Z to select rotation axis" << std::endl;
    std::cout << "Then DRAG MOUSE to rotate the ball!" << std::endl;
    std::cout << "Press T to switch to translation mode" << std::endl;
    std::cout << "Press R to reset ball" << std::endl;
    std::cout << "================================" << std::endl;
    
    glutMainLoop();
    
    return 0;
}