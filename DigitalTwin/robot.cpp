#include <GL/glut.h>
#include <cmath>
#include <string>

// ── window ──────────────────────────────────────────────────
static int  W = 1200, H = 800;
static const char* TITLE = "Differential Drive Robot OpenGL";

// ── camera ──────────────────────────────────────────────────
static float camTheta  =  35.0f;   // azimuth  (deg)
static float camPhi    =  25.0f;   // elevation (deg)
static float camDist   =  18.0f;
static int   lastX = 0, lastY = 0;
static bool  dragging  = false;

// ── robot pose ──────────────────────────────────────────────
static float robotX   =  0.0f;
static float robotZ   =  0.0f;
static float robotYaw =  0.0f;   // degrees around Y

// ── robot geometry ──────────────────────────────────────────
static const float BODY_W  = 2.0f;   // X
static const float BODY_H  = 2.0f;   // Y
static const float BODY_D  = 2.0f;   // Z

static const float WHEEL_RADIUS = 0.5f; 
static const float WHEEL_THICKNESS = 0.25f;
static const int   WHEEL_SLICES = 100;

// wheel axle offset: wheels sit flush to body sides
static const float WHEEL_OFFSET_Y = 0.0f;   // height: wheel centre at ground + R
static const float WHEEL_AXLE_X   = BODY_W / 2.0f + WHEEL_THICKNESS / 2.0f;

// ── animation ───────────────────────────────────────────────
static float wheelAngle = 0.0f;
static float leftSpeed  = 0.0f;   // deg / frame
static float rightSpeed = 0.0f;

// ── grid ────────────────────────────────────────────────────
static const int   GRID_HALF = 12;
static const float GRID_STEP = 1.0f;

// ────────────────────────────────────────────────────────────
//  helpers
// ────────────────────────────────────────────────────────────
static void setColor(float r, float g, float b, float a = 1.f)
{
    glColor4f(r, g, b, a);
}

// Draw a cylinder (open) aligned along X axis, centred at origin
static void drawCylinderX(float radius, float length, int slices)
{
    float half = length * 0.5f;
    glBegin(GL_TRIANGLE_STRIP);
    for (int i = 0; i <= slices; ++i) {
        float ang = (float)i / slices * 2.0f * (float)M_PI;
        float y   = radius * cosf(ang);
        float z   = radius * sinf(ang);
        glNormal3f(0, cosf(ang), sinf(ang));
        glVertex3f(-half, y, z);
        glVertex3f( half, y, z);
    }
    glEnd();
}

// Draw a filled disk (cap) facing ±X
static void drawDiskX(float radius, float x, int slices, bool flip)
{
    glBegin(GL_TRIANGLE_FAN);
    glNormal3f(flip ? -1.f : 1.f, 0, 0);
    glVertex3f(x, 0, 0);
    int n = flip ? slices : 0;
    int d = flip ? -1 : 1;
    for (int i = 0; i <= slices; ++i) {
        int idx = n + d * i;
        float ang = (float)idx / slices * 2.0f * (float)M_PI;
        glVertex3f(x, radius * cosf(ang), radius * sinf(ang));
    }
    glEnd();
}

// Solid wheel: cylinder + 2 caps, axis = X
static void drawWheel(float radius, float thickness, int slices)
{
    drawCylinderX(radius, thickness, slices);
    drawDiskX(radius, -thickness * 0.5f, slices, true);
    drawDiskX(radius,  thickness * 0.5f, slices, false);
}

// ────────────────────────────────────────────────────────────
//  Axis arrow  (TF-style):  X=red  Y=green  Z=blue
// ────────────────────────────────────────────────────────────
static void drawAxisArrow(float len, float headLen, float r)
{
    float bodyLen = len - headLen;
    float headR   = r * 2.5f;

    // shaft
    GLUquadric* q = gluNewQuadric();
    gluCylinder(q, r, r, bodyLen, 12, 1);

    // cone tip
    // glPushMatrix();
    // glTranslatef(0, 0, bodyLen);
    // gluCylinder(q, headR, 0.0f, headLen, 12, 1);
    // glPopMatrix();

    gluDeleteQuadric(q);
}

static void drawAxes(float scale)
{
    glLineWidth(2.f);
    glPushAttrib(GL_LIGHTING_BIT);
    glDisable(GL_LIGHTING);

    // +X  red
    setColor(1, 0, 0);
    glPushMatrix();
    glRotatef(90, 0, 1, 0);
    drawAxisArrow(scale, scale * 0.25f, scale * 0.025f);
    glPopMatrix();

    // +Y  green
    setColor(0, 0.85f, 0);
    glPushMatrix();
    glRotatef(-90, 1, 0, 0);
    drawAxisArrow(scale, scale * 0.25f, scale * 0.025f);
    glPopMatrix();

    // +Z  blue
    setColor(0.2f, 0.4f, 1.0f);
    drawAxisArrow(scale, scale * 0.25f, scale * 0.025f);

    glPopAttrib();
}

// ────────────────────────────────────────────────────────────
//  Grid (XZ plane, y = 0)
// ────────────────────────────────────────────────────────────
static void drawGrid()
{
    glPushAttrib(GL_LIGHTING_BIT | GL_LINE_BIT | GL_COLOR_BUFFER_BIT);
    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glLineWidth(1.0f);
    glBegin(GL_LINES);
    for (int i = -GRID_HALF; i <= GRID_HALF; ++i) {
        float fi = (float)i * GRID_STEP;
        bool  major = (i % 5 == 0);
        if (major)
            glColor4f(0.55f, 0.55f, 0.55f, 0.9f);
        else
            glColor4f(0.35f, 0.35f, 0.35f, 0.5f);

        // along Z
        glVertex3f(fi, 0, -(float)GRID_HALF);
        glVertex3f(fi, 0,  (float)GRID_HALF);
        // along X
        glVertex3f(-(float)GRID_HALF, 0, fi);
        glVertex3f( (float)GRID_HALF, 0, fi);
    }
    glEnd();

    glPopAttrib();
}

// ────────────────────────────────────────────────────────────
//  Robot body  –  textured cuboid (faces different colours)
// ────────────────────────────────────────────────────────────
static void drawRobotBody()
{
    float w = BODY_W * 0.5f;
    float h = BODY_H * 0.5f;
    float d = BODY_D * 0.5f;

    // offset so bottom of body rests on wheel centre height (y = WHEEL_RADIUS)
    float baseY = WHEEL_RADIUS;

    glPushMatrix();
    glTranslatef(0, baseY + h, 0);  // centre of body in world Y

    // // front face  +Z  (orange)
    // setColor(1.0f, 0.55f, 0.1f);
    // glBegin(GL_QUADS);
    // glNormal3f( 0,  0,  1);
    // glVertex3f(-w, -h,  d);
    // glVertex3f( w, -h,  d);
    // glVertex3f( w,  h,  d);
    // glVertex3f(-w,  h,  d);
    // glEnd();

    // // back face  –Z  (dark orange)
    // setColor(0.7f, 0.35f, 0.05f);
    // glBegin(GL_QUADS);
    // glNormal3f( 0,  0, -1);
    // glVertex3f( w, -h, -d);
    // glVertex3f(-w, -h, -d);
    // glVertex3f(-w,  h, -d);
    // glVertex3f( w,  h, -d);
    // glEnd();

    // // left face  –X  (teal)
    // setColor(0.1f, 0.75f, 0.75f);
    // glBegin(GL_QUADS);
    // glNormal3f(-1,  0,  0);
    // glVertex3f(-w, -h, -d);
    // glVertex3f(-w, -h,  d);
    // glVertex3f(-w,  h,  d);
    // glVertex3f(-w,  h, -d);
    // glEnd();

    // // right face  +X  (teal-dark)
    // setColor(0.05f, 0.5f, 0.5f);
    // glBegin(GL_QUADS);
    // glNormal3f( 1,  0,  0);
    // glVertex3f( w, -h,  d);
    // glVertex3f( w, -h, -d);
    // glVertex3f( w,  h, -d);
    // glVertex3f( w,  h,  d);
    // glEnd();

    // // top face  +Y  (light grey)
    // setColor(0.85f, 0.85f, 0.85f);
    // glBegin(GL_QUADS);
    // glNormal3f( 0,  1,  0);
    // glVertex3f(-w,  h,  d);
    // glVertex3f( w,  h,  d);
    // glVertex3f( w,  h, -d);
    // glVertex3f(-w,  h, -d);
    // glEnd();

    // bottom face  –Y  (dark grey)
    setColor(0.3f, 0.3f, 0.3f);
    glBegin(GL_QUADS);
    glNormal3f( 0, -1,  0);
    glVertex3f(-w, -h, -d);
    glVertex3f( w, -h, -d);
    glVertex3f( w, -h,  d);
    glVertex3f(-w, -h,  d);
    glEnd();

   
    
    glEnd();
    glPopAttrib();

    glPopMatrix();
}

// ────────────────────────────────────────────────────────────
//  Full robot  (body + 2 diff-drive wheels)
// ────────────────────────────────────────────────────────────
static void drawRobot()
{
    // ── body ────────────────────────────────────────────────
    drawRobotBody();

    // ── left wheel  (–X side) ───────────────────────────────
    setColor(0.15f, 0.15f, 0.15f);
    glPushMatrix();
    glTranslatef(-WHEEL_AXLE_X, WHEEL_RADIUS, 0);
    glRotatef(wheelAngle, 1, 0, 0);     // spin
    drawWheel(WHEEL_RADIUS, WHEEL_THICKNESS, WHEEL_SLICES);
    glPopMatrix();

    // left wheel tyre highlight (thin grey ring)
    setColor(0.4f, 0.4f, 0.4f);
    glPushMatrix();
    glTranslatef(-WHEEL_AXLE_X, WHEEL_RADIUS, 0);
    glRotatef(wheelAngle, 1, 0, 0);
    glPushAttrib(GL_LIGHTING_BIT);
    glDisable(GL_LIGHTING);
    glLineWidth(2.f);
    glBegin(GL_LINE_LOOP);
    for (int i = 0; i < WHEEL_SLICES; ++i) {
        float a = (float)i / WHEEL_SLICES * 2.f * (float)M_PI;
        glVertex3f(-WHEEL_THICKNESS * 0.5f, WHEEL_RADIUS * cosf(a), WHEEL_RADIUS * sinf(a));
    }
    glEnd();
    glPopAttrib();
    glPopMatrix();

    // ── right wheel  (+X side) ──────────────────────────────
    setColor(0.15f, 0.15f, 0.15f);
    glPushMatrix();
    glTranslatef( WHEEL_AXLE_X, WHEEL_RADIUS, 0);
    glRotatef(wheelAngle, 1, 0, 0);
    drawWheel(WHEEL_RADIUS, WHEEL_THICKNESS, WHEEL_SLICES);
    glPopMatrix();

    setColor(0.4f, 0.4f, 0.4f);
    glPushMatrix();
    glTranslatef( WHEEL_AXLE_X, WHEEL_RADIUS, 0);
    glRotatef(wheelAngle, 1, 0, 0);
    glPushAttrib(GL_LIGHTING_BIT);
    glDisable(GL_LIGHTING);
    glLineWidth(2.f);
    glBegin(GL_LINE_LOOP);
    for (int i = 0; i < WHEEL_SLICES; ++i) {
        float a = (float)i / WHEEL_SLICES * 2.f * (float)M_PI;
        glVertex3f( WHEEL_THICKNESS * 0.5f, WHEEL_RADIUS * cosf(a), WHEEL_RADIUS * sinf(a));
    }
    glEnd();
    glPopAttrib();
    glPopMatrix();

    // ── body-frame axes  (TF) ───────────────────────────────
    glPushMatrix();
    glTranslatef(0, WHEEL_RADIUS, 0);        // origin at ground-centre of robot
    drawAxes(2.0f);
    glPopMatrix();
}

// ────────────────────────────────────────────────────────────
//  HUD label helper
// ────────────────────────────────────────────────────────────
static void renderText(float x, float y, const std::string& txt,
                        float r, float g, float b)
{
    glMatrixMode(GL_PROJECTION);
    glPushMatrix(); glLoadIdentity();
    gluOrtho2D(0, W, 0, H);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix(); glLoadIdentity();
    glDisable(GL_LIGHTING);
    glColor3f(r, g, b);
    glRasterPos2f(x, y);
    for (char c : txt)
        glutBitmapCharacter(GLUT_BITMAP_9_BY_15, c);
    glEnable(GL_LIGHTING);
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}

// ────────────────────────────────────────────────────────────
//  Display
// ────────────────────────────────────────────────────────────
static void display()
{
    glClearColor(0.08f, 0.08f, 0.10f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // ── camera ──────────────────────────────────────────────
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, (double)W / H, 0.1, 200.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    float radT = camTheta * (float)M_PI / 180.f;
    float radP = camPhi   * (float)M_PI / 180.f;
    float cx = camDist * cosf(radP) * sinf(radT);
    float cy = camDist * sinf(radP);
    float cz = camDist * cosf(radP) * cosf(radT);
    gluLookAt(cx, cy, cz, 0, 1.5f, 0, 0, 1, 0);

    // ── lighting ────────────────────────────────────────────
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHT1);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_NORMALIZE);

    GLfloat amb[]  = {0.25f,0.25f,0.25f,1.f};
    GLfloat diff[] = {1.0f, 0.95f,0.85f,1.f};
    GLfloat pos0[] = {8.f, 12.f, 8.f, 1.f};
    GLfloat pos1[] = {-6.f, 8.f, -6.f, 1.f};
    GLfloat diff1[]= {0.3f, 0.4f, 0.6f, 1.f};

    glLightfv(GL_LIGHT0, GL_AMBIENT,  amb);
    glLightfv(GL_LIGHT0, GL_DIFFUSE,  diff);
    glLightfv(GL_LIGHT0, GL_POSITION, pos0);
    glLightfv(GL_LIGHT1, GL_DIFFUSE,  diff1);
    glLightfv(GL_LIGHT1, GL_POSITION, pos1);

    GLfloat matSpec[]  = {0.3f,0.3f,0.3f,1.f};
    GLfloat matShine[] = {32.f};
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR,  matSpec);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, matShine);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    glEnable(GL_COLOR_MATERIAL);

    // ── world-frame axes ────────────────────────────────────
    drawAxes(3.0f);

    // ── grid ────────────────────────────────────────────────
    drawGrid();

    // ── robot ───────────────────────────────────────────────
    glPushMatrix();
    glTranslatef(robotX, 0, robotZ);
    glRotatef(robotYaw, 0, 1, 0);
    drawRobot();
    glPopMatrix();

    // // ── HUD ─────────────────────────────────────────────────
    // renderText(10, H - 22,
    //     "Diff-Drive Robot  |  Body 2x2x2  |  WheelR=1.5  T=0.5",
    //     0.9f, 0.9f, 0.9f);
    // renderText(10, H - 42,
    //     "Drag to orbit  |  Scroll to zoom  |  WASD steer  |  Q/E yaw  |  R reset",
    //     0.6f, 0.6f, 0.6f);

    // char buf[128];
    // snprintf(buf, sizeof(buf),
    //     "Pos (%.2f, %.2f)  Yaw %.1f°  WheelAngle %.1f°",
    //     robotX, robotZ, robotYaw, wheelAngle);
    // renderText(10, 10, buf, 0.7f, 0.9f, 0.7f);

    // // axis labels near world origin
    // renderText(W/2 + 30, H/2, "Z", 0.2f, 0.4f, 1.0f);

    glutSwapBuffers();
}

// ────────────────────────────────────────────────────────────
//  Idle / animation
// ────────────────────────────────────────────────────────────
static void idle()
{
    wheelAngle += (leftSpeed + rightSpeed) * 0.5f * 0.1f;
    if (wheelAngle > 360.f) wheelAngle -= 360.f;
    glutPostRedisplay();
}

// ────────────────────────────────────────────────────────────
//  Keyboard
// ────────────────────────────────────────────────────────────
static void keyboard(unsigned char key, int /*x*/, int /*y*/)
{
    float rad = robotYaw * (float)M_PI / 180.f;
    float fwd = 0.15f;
    switch (key) {
    case 'w': case 'W':
        robotX += sinf(rad) * fwd;
        robotZ += cosf(rad) * fwd;
        break;
    case 's': case 'S':
        robotX -= sinf(rad) * fwd;
        robotZ -= cosf(rad) * fwd;
        break;
    case 'a': case 'A':
        robotX -= cosf(rad) * fwd;
        robotZ += sinf(rad) * fwd;
        break;
    case 'd': case 'D':
        robotX += cosf(rad) * fwd;
        robotZ -= sinf(rad) * fwd;
        break;
    case 'q': case 'Q': robotYaw += 2.f; break;
    case 'e': case 'E': robotYaw -= 2.f; break;
    case 'r': case 'R':
        robotX = robotZ = robotYaw = 0.f;
        camTheta = 35.f; camPhi = 25.f; camDist = 18.f;
        break;
    case 27: exit(0);  // ESC
    }
    glutPostRedisplay();
}

// ────────────────────────────────────────────────────────────
//  Mouse
// ────────────────────────────────────────────────────────────
static void mouse(int btn, int state, int x, int y)
{
    if (btn == GLUT_LEFT_BUTTON) {
        dragging = (state == GLUT_DOWN);
        lastX = x; lastY = y;
    }
    // scroll to zoom
    if (btn == 3) { camDist -= 0.5f; if (camDist < 2.f) camDist = 2.f; }
    if (btn == 4) { camDist += 0.5f; if (camDist > 80.f) camDist = 80.f; }
    glutPostRedisplay();
}

static void motion(int x, int y)
{
    if (dragging) {
        camTheta -= (x - lastX) * 0.5f;
        camPhi   += (y - lastY) * 0.5f;
        if (camPhi >  89.f) camPhi =  89.f;
        if (camPhi < -89.f) camPhi = -89.f;
        lastX = x; lastY = y;
        glutPostRedisplay();
    }
}

// ────────────────────────────────────────────────────────────
//  Reshape
// ────────────────────────────────────────────────────────────
static void reshape(int w, int h)
{
    W = w; H = (h > 0 ? h : 1);
    glViewport(0, 0, W, H);
}

// ────────────────────────────────────────────────────────────
//  Main
// ────────────────────────────────────────────────────────────
int main(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(W, H);
    glutCreateWindow(TITLE);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutMouseFunc(mouse);
    glutMotionFunc(motion);
    glutIdleFunc(idle);

    glutMainLoop();
    return 0;
}