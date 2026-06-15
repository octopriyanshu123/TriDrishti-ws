#include <GL/glut.h>
#include <cmath>
#include <cstdlib>
#include <linux/joystick.h>
#include <fcntl.h>
#include <unistd.h>
#include <chrono>
#include <iostream>
#include <cstring>

// Joystick variables
static int js_fd = -1;
static float axis[8] = {0};
static std::chrono::steady_clock::time_point lastTime;

// Camera variables - Fixed at (0,15,15) but with orientation control
float cameraX = 0.0f, cameraY = 15.0f, cameraZ = 15.0f;
float lookX = 0.0f, lookY = 0.0f, lookZ = 0.0f;

// Camera orientation angles (Euler angles)
float cameraAngleX = 0.0f;  // Rotation around X axis (pitch)
float cameraAngleY = 0.0f;  // Rotation around Y axis (yaw)
float cameraAngleZ = 0.0f;  // Rotation around Z axis (roll)

// Camera position offset for movement
float cameraOffsetX = 0.0f;
float cameraOffsetY = 0.0f;
float cameraOffsetZ = 0.0f;

// Window dimensions
int windowWidth = 1024;
int windowHeight = 768;

// Movement speed
float moveSpeed = 2.0f;
float rotationSpeed = 50.0f;

// Joystick deadzone
const float DEADZONE = 0.1f;

// Draw UI text
void drawText(float x, float y, const char* text) {
    glDisable(GL_LIGHTING);
    glColor3f(1.0f, 1.0f, 1.0f);
    glRasterPos2f(x, y);
    for (const char* c = text; *c != '\0'; c++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
    }
    glEnable(GL_LIGHTING);
}
// Axis mapping
// axis[0] = Left Stick X (usually)
// axis[1] = Left Stick Y (usually)
// axis[2] = Right Stick X (usually)
// axis[3] = Right Stick Y (usually)
// axis[4] = Left Trigger
// axis[5] = Right Trigger
// axis[6] = D-pad X
// axis[7] = D-pad Y

// Function declarations
void joyAxisCallback(int axisNum, float value);
void joyButtonCallback(int buttonNum, int value);
void updatePoseFromJoystick();
static bool initJoystick();
void updateCameraFromJoystick();
void drawGrid();
void drawAxes();
void drawOriginMarker();
void display();
void reshape(int w, int h);
void keyboard(unsigned char key, int x, int y);
void specialKeys(int key, int x, int y);
void initOpenGL();
void updateCameraVectors();

// Update camera position and orientation based on joystick input
void updateCameraFromJoystick() {
    auto now = std::chrono::steady_clock::now();
    float deltaTime = std::chrono::duration<float>(now - lastTime).count();
    lastTime = now;
    
    // Limit deltaTime to prevent large jumps
    if (deltaTime > 0.033f) deltaTime = 0.033f;
    
    // Get current time for smooth movement
    static float lastFrameTime = 0;
    
    // --- Camera Movement using axis 1, 2, 3 ---
    // axis[0] = X movement (left/right)
    // axis[1] = Y movement (up/down)
    // axis[2] = Z movement (forward/backward)
    
    float moveX = 0.0f;
    float moveY = 0.0f;
    float moveZ = 0.0f;
    
    // Apply deadzone to axis values
    if (fabs(axis[0]) > DEADZONE) moveX = axis[0];
    if (fabs(axis[1]) > DEADZONE) moveY = -axis[1];  // Inverted Y
    if (fabs(axis[2]) > DEADZONE) moveZ = axis[2];
    
    // Apply movement with speed
    float speed = moveSpeed * deltaTime;
    cameraOffsetX += moveX * speed;
    cameraOffsetY += moveY * speed;
    cameraOffsetZ += moveZ * speed;
    
    // Clamp camera position to reasonable bounds
    if (cameraOffsetX > 15.0f) cameraOffsetX = 15.0f;
    if (cameraOffsetX < -15.0f) cameraOffsetX = -15.0f;
    if (cameraOffsetY > 10.0f) cameraOffsetY = 10.0f;
    if (cameraOffsetY < -5.0f) cameraOffsetY = -5.0f;
    if (cameraOffsetZ > 15.0f) cameraOffsetZ = 15.0f;
    if (cameraOffsetZ < -15.0f) cameraOffsetZ = -15.0f;
    
    // --- Camera Orientation using axis 4, 5, 6 ---
    // axis[3] = X rotation (pitch)
    // axis[4] = Y rotation (yaw)
    // axis[5] = Z rotation (roll)
    
    float rotX = 0.0f;
    float rotY = 0.0f;
    float rotZ = 0.0f;
    
    if (fabs(axis[3]) > DEADZONE) rotX = axis[3] * rotationSpeed * deltaTime;
    if (fabs(axis[4]) > DEADZONE) rotY = axis[4] * rotationSpeed * deltaTime;
    if (fabs(axis[5]) > DEADZONE) rotZ = axis[5] * rotationSpeed * deltaTime;
    
    // Apply rotation
    // cameraAngleX += rotX;
    // cameraAngleY += rotY;
    // cameraAngleZ += rotZ;
    
    // Clamp angles to prevent gimbal lock
    // if (cameraAngleX > 89.0f) cameraAngleX = 89.0f;
    // if (cameraAngleX < -89.0f) cameraAngleX = -89.0f;
    
    // // Keep yaw and roll in range
    // while (cameraAngleY >= 360.0f) cameraAngleY -= 360.0f;
    // while (cameraAngleY < 0.0f) cameraAngleY += 360.0f;
    // while (cameraAngleZ >= 360.0f) cameraAngleZ -= 360.0f;
    // while (cameraAngleZ < 0.0f) cameraAngleZ += 360.0f;
    
    // Update look direction based on orientation
    updateCameraVectors();
}

// Update camera look-at point based on orientation angles
void updateCameraVectors() {
    // Calculate direction vector from Euler angles
    // Convert degrees to radians
    float pitchRad = cameraAngleX * 3.14159f / 180.0f;
    float yawRad = cameraAngleY * 3.14159f / 180.0f;
    
    // Calculate look direction
    lookX = cos(yawRad) * cos(pitchRad);
    lookY = sin(pitchRad);
    lookZ = sin(yawRad) * cos(pitchRad);
    
    // Normalize look direction
    float len = sqrt(lookX*lookX + lookY*lookY + lookZ*lookZ);
    if (len > 0.001f) {
        lookX /= len;
        lookY /= len;
        lookZ /= len;
    }
    
    // Add the look point to camera position
    lookX += cameraX + cameraOffsetX;
    lookY += cameraY + cameraOffsetY;
    lookZ += cameraZ + cameraOffsetZ;
}

// Axis callback for joystick
void joyAxisCallback(int axisNum, float value) {
    if (axisNum < 8) {
        axis[axisNum] = value;
    }
    char info[256];

    sprintf(info, "Joystick Axis: [0:%.2f] [1:%.2f] [2:%.2f] [3:%.2f] [4:%.2f] [5:%.2f]", 
            axis[0], axis[1], axis[2], axis[3], axis[4], axis[5]);
    drawText(10, 30, info);
}

// Button callback for joystick
void joyButtonCallback(int buttonNum, int value) {
    switch(buttonNum) {
        case 0: // Button A - Reset camera
            if (value == 1) {
                cameraOffsetX = 0.0f;
                cameraOffsetY = 0.0f;
                cameraOffsetZ = 0.0f;
                cameraAngleX = 0.0f;
                cameraAngleY = 0.0f;
                cameraAngleZ = 0.0f;
                updateCameraVectors();
                std::cout << "Camera reset to default position and orientation" << std::endl;
            }
            break;
        case 1: // Button B - Toggle grid
            break;
        case 2: // Button X - Print status
            if (value == 1) {
                std::cout << "Camera Position Offset: (" << cameraOffsetX 
                          << ", " << cameraOffsetY << ", " << cameraOffsetZ << ")" << std::endl;
                std::cout << "Camera Angles: (" << cameraAngleX 
                          << ", " << cameraAngleY << ", " << cameraAngleZ << ")" << std::endl;
            }
            break;
        case 3: // Button Y - Reset orientation only
            if (value == 1) {
                cameraAngleX = 0.0f;
                cameraAngleY = 0.0f;
                cameraAngleZ = 0.0f;
                updateCameraVectors();
                std::cout << "Camera orientation reset" << std::endl;
            }
            break;
    }
}

// Update joystick state
void updatePoseFromJoystick() {
    if (js_fd < 0) return;
    
    js_event event;
    while (read(js_fd, &event, sizeof(event)) > 0) {
        event.type &= ~JS_EVENT_INIT;
        
        if (event.type == JS_EVENT_AXIS) {
            if (event.number < 8) {
                float value = static_cast<float>(event.value) / 32767.0f;
                axis[event.number] = value;
                joyAxisCallback(event.number, value);
            }
        }
        else if (event.type == JS_EVENT_BUTTON) {
            joyButtonCallback(event.number, event.value);
        }
        
    }

}

// Idle callback
static void idle() {
    updatePoseFromJoystick();
    updateCameraFromJoystick();
    glutPostRedisplay();
}

// Initialize joystick
static bool initJoystick() {
    js_fd = open("/dev/input/js0", O_RDONLY | O_NONBLOCK);
    
    if (js_fd < 0) {
        std::cout << "Failed to open /dev/input/js0" << std::endl;
        std::cout << "Make sure joystick is connected and you have permissions" << std::endl;
        std::cout << "Try: sudo chmod 666 /dev/input/js0" << std::endl;
        return false;
    }
    
    lastTime = std::chrono::steady_clock::now();
    std::cout << "Joystick initialized successfully!" << std::endl;
    return true;
}

// Draw grid
void drawGrid() {
    glColor3f(0.5f, 0.5f, 0.5f);
    glBegin(GL_LINES);
    
    // Draw grid lines from -15 to 15
    for(float i = -15.0f; i <= 15.0f; i += 1.0f) {
        // Lines parallel to X axis
        glVertex3f(-15.0f, -1.0f, i);
        glVertex3f(15.0f, -1.0f, i);
        
        // Lines parallel to Z axis
        glVertex3f(i, -1.0f, -15.0f);
        glVertex3f(i, -1.0f, 15.0f);
    }
    
    glEnd();
    
    // Draw X axis line (red)
    glColor3f(1.0f, 0.0f, 0.0f);
    glBegin(GL_LINES);
    glVertex3f(-15.0f, -1.0f, 0.0f);
    glVertex3f(15.0f, -1.0f, 0.0f);
    glEnd();
    
    // Draw Z axis line (blue)
    glColor3f(0.0f, 0.0f, 1.0f);
    glBegin(GL_LINES);
    glVertex3f(0.0f, -1.0f, -15.0f);
    glVertex3f(0.0f, -1.0f, 15.0f);
    glEnd();
}

// Draw axes helper (simplified version since Axis.cpp might not exist)
void drawAxes() {
    glLineWidth(3.0f);
    
    // X axis (red)
    glColor3f(1.0f, 0.0f, 0.0f);
    glBegin(GL_LINES);
    glVertex3f(-3.0f, 0.0f, 0.0f);
    glVertex3f(3.0f, 0.0f, 0.0f);
    glEnd();
    
    // Arrow for X
    glPushMatrix();
    glTranslatef(3.0f, 0.0f, 0.0f);
    glColor3f(1.0f, 0.0f, 0.0f);
    glutSolidCone(0.1f, 0.3f, 10, 10);
    glPopMatrix();
    
    // Y axis (green)
    glColor3f(0.0f, 1.0f, 0.0f);
    glBegin(GL_LINES);
    glVertex3f(0.0f, -3.0f, 0.0f);
    glVertex3f(0.0f, 3.0f, 0.0f);
    glEnd();
    
    // Arrow for Y
    glPushMatrix();
    glTranslatef(0.0f, 3.0f, 0.0f);
    glColor3f(0.0f, 1.0f, 0.0f);
    glutSolidCone(0.1f, 0.3f, 10, 10);
    glPopMatrix();
    
    // Z axis (blue)
    glColor3f(0.0f, 0.0f, 1.0f);
    glBegin(GL_LINES);
    glVertex3f(0.0f, 0.0f, -3.0f);
    glVertex3f(0.0f, 0.0f, 3.0f);
    glEnd();
    
    // Arrow for Z
    glPushMatrix();
    glTranslatef(0.0f, 0.0f, 3.0f);
    glColor3f(0.0f, 0.0f, 1.0f);
    glutSolidCone(0.1f, 0.3f, 10, 10);
    glPopMatrix();
    
    glLineWidth(1.0f);
}

// Draw origin marker
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
    
    // Set camera position and orientation
    gluLookAt(cameraX + cameraOffsetX, 
              cameraY + cameraOffsetY, 
              cameraZ + cameraOffsetZ,
              lookX, lookY, lookZ,
              0.0f, 1.0f, 0.0f);
    
    // Draw grid
    drawGrid();
    
    // Draw axes
    drawAxes();
    
    // Draw origin marker
    drawOriginMarker();
    
    // Draw UI (2D overlay)
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
    sprintf(info, "Camera Position Offset: (%.2f, %.2f, %.2f)", cameraOffsetX, cameraOffsetY, cameraOffsetZ);
    drawText(10, glutGet(GLUT_WINDOW_HEIGHT) - 30, info);
    
    sprintf(info, "Camera Orientation: Pitch=%.1f Yaw=%.1f Roll=%.1f", cameraAngleX, cameraAngleY, cameraAngleZ);
    drawText(10, glutGet(GLUT_WINDOW_HEIGHT) - 55, info);
    
    drawText(10, glutGet(GLUT_WINDOW_HEIGHT) - 85, "Joystick Controls:");
    drawText(10, glutGet(GLUT_WINDOW_HEIGHT) - 105, "Axis 0: Move Left/Right");
    drawText(10, glutGet(GLUT_WINDOW_HEIGHT) - 125, "Axis 1: Move Up/Down");
    drawText(10, glutGet(GLUT_WINDOW_HEIGHT) - 145, "Axis 2: Move Forward/Backward");
    drawText(10, glutGet(GLUT_WINDOW_HEIGHT) - 165, "Axis 3: Rotate X (Pitch)");
    drawText(10, glutGet(GLUT_WINDOW_HEIGHT) - 185, "Axis 4: Rotate Y (Yaw)");
    drawText(10, glutGet(GLUT_WINDOW_HEIGHT) - 205, "Axis 5: Rotate Z (Roll)");
    drawText(10, glutGet(GLUT_WINDOW_HEIGHT) - 225, "Button A: Reset Camera");
    drawText(10, glutGet(GLUT_WINDOW_HEIGHT) - 245, "Button Y: Reset Orientation");
    
    // Display joystick axis values
    // sprintf(info, "Joystick Axis: [0:%.2f] [1:%.2f] [2:%.2f] [3:%.2f] [4:%.2f] [5:%.2f]", 
    //         axis[0], axis[1], axis[2], axis[3], axis[4], axis[5]);
    // drawText(10, 30, info);
    
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
    windowWidth = w;
    windowHeight = h;
    
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0f, (float)w / (float)h, 0.1f, 100.0f);
    glMatrixMode(GL_MODELVIEW);
}

// Keyboard callback
void keyboard(unsigned char key, int x, int y) {
    switch(key) {
        case 'r':
        case 'R':
            cameraOffsetX = 0.0f;
            cameraOffsetY = 0.0f;
            cameraOffsetZ = 0.0f;
            cameraAngleX = 0.0f;
            cameraAngleY = 0.0f;
            cameraAngleZ = 0.0f;
            updateCameraVectors();
            std::cout << "Camera reset" << std::endl;
            break;
        case 27: // ESC
            if (js_fd >= 0) close(js_fd);
            exit(0);
            break;
    }
    glutPostRedisplay();
}

// Special keyboard callback
void specialKeys(int key, int x, int y) {
    float step = 0.5f;
    switch(key) {
        case GLUT_KEY_UP:
            cameraOffsetZ -= step;
            break;
        case GLUT_KEY_DOWN:
            cameraOffsetZ += step;
            break;
        case GLUT_KEY_LEFT:
            cameraOffsetX -= step;
            break;
        case GLUT_KEY_RIGHT:
            cameraOffsetX += step;
            break;
        case GLUT_KEY_PAGE_UP:
            cameraOffsetY += step;
            break;
        case GLUT_KEY_PAGE_DOWN:
            cameraOffsetY -= step;
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
    
    // Initialize camera vectors
    updateCameraVectors();
}

int main(int argc, char** argv) {
    // Initialize joystick (optional - continue even if no joystick)
    if (!initJoystick()) {
        std::cout << "Continuing without joystick support..." << std::endl;
        std::cout << "Using keyboard controls instead" << std::endl;
    }
    
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(windowWidth, windowHeight);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("3D Space with Joystick Camera Control");
    
    initOpenGL();
    
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(specialKeys);
    
    if (js_fd >= 0) {
        glutIdleFunc(idle);
        std::cout << "\n=== JOYSTICK CONTROL ACTIVE ===" << std::endl;
        std::cout << "Axis 0: Move Left/Right" << std::endl;
        std::cout << "Axis 1: Move Up/Down" << std::endl;
        std::cout << "Axis 2: Move Forward/Backward" << std::endl;
        std::cout << "Axis 3: Pitch (Look Up/Down)" << std::endl;
        std::cout << "Axis 4: Yaw (Look Left/Right)" << std::endl;
        std::cout << "Axis 5: Roll" << std::endl;
        std::cout << "Button A: Reset Camera" << std::endl;
        std::cout << "===============================\n" << std::endl;
    } else {
        std::cout << "\n=== KEYBOARD CONTROLS ===" << std::endl;
        std::cout << "Arrow Keys: Move Camera" << std::endl;
        std::cout << "Page Up/Down: Move Up/Down" << std::endl;
        std::cout << "R: Reset Camera" << std::endl;
        std::cout << "ESC: Exit" << std::endl;
        std::cout << "========================\n" << std::endl;
    }
    
    glutMainLoop();
    
    if (js_fd >= 0) close(js_fd);
    return 0;
}
