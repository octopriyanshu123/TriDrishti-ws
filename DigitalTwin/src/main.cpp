#include <GL/glut.h>
#include <cmath>
#include <cstdio>
#include <linux/joystick.h>
#include <fcntl.h>
#include <unistd.h>
#include <chrono>
#include <iostream>

#include "core/Tank.cpp"
#include "core/Robot.cpp"
#include "core/FlatRobot.cpp"
#include "core/Pose.cpp"


static int js_fd = -1;
static float axis[8] = {0};
static std::chrono::steady_clock::time_point lastTime;
GLUquadric *quad = NULL;

static double theta = 1.0;
static double height = 2.0;

static Pose robotPose;
static Pose spotPose;



// ── window ───────────────────────────────────────────────────
static int W = 1280, H = 800;

// ── scene objects ────────────────────────────────────────────
static Tank tank;
static FlatRobot robot;

// ── camera ───────────────────────────────────────────────────
static float camTheta = 90.f, camPhi = 20.f, camDist = 75.f;
static int lastX = 0, lastY = 0;
static bool dragging = false;

// ── world-frame axis arrows ──────────────────────────────────
static void drawArrow(float len)
{
    GLUquadric *q = gluNewQuadric();
    gluCylinder(q, 0.08f, 0.08f, len * 0.8f, 8, 1);
    glPushMatrix();
    glTranslatef(0, 0, len * 0.8f);
    gluCylinder(q, 0.20f, 0.f, len * 0.2f, 8, 1);
    glPopMatrix();
    gluDeleteQuadric(q);
}

void drawTank()
{
    // Tank body (cylinder)
    tank.draw();
}
static void drawWorldAxes()
{
    glPushAttrib(GL_LIGHTING_BIT);
    glDisable(GL_LIGHTING);
    glColor3f(1.f, 0.1f, 0.1f);
    glPushMatrix();
    glRotatef(90, 0, 1, 0);
    drawArrow(5.f);
    glPopMatrix();
    glColor3f(0.1f, 1.f, 0.1f);
    glPushMatrix();
    glRotatef(-90, 1, 0, 0);
    drawArrow(5.f);
    glPopMatrix();
    glColor3f(0.2f, 0.4f, 1.f);
    drawArrow(5.f);
    glPopAttrib();
}

static void drawGrid()
{
    glPushAttrib(GL_LIGHTING_BIT);
    glDisable(GL_LIGHTING);
    glBegin(GL_LINES);
    for (int i = -12; i <= 12; i++)
    {
        float f = (float)i;
        bool maj = (i % 4 == 0);
        if (maj)
            glColor4f(.50f, .50f, .52f, .8f);
        else
            glColor4f(.28f, .28f, .30f, .4f);
        glVertex3f(f, 0, -12);
        glVertex3f(f, 0, 12);
        glVertex3f(-12, 0, f);
        glVertex3f(12, 0, f);
    }
    glEnd();
    glPopAttrib();
}

// ── lighting setup ───────────────────────────────────────────
static void setupLighting()
{
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHT1);
    glEnable(GL_NORMALIZE);

    GLfloat p0[] = {15, 20, 15, 1}, d0[] = {1, .95f, .85f, 1}, a0[] = {.22f, .22f, .22f, 1};
    GLfloat p1[] = {-10, 10, -10, 1}, d1[] = {.3f, .4f, .6f, 1};
    glLightfv(GL_LIGHT0, GL_POSITION, p0);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, d0);
    glLightfv(GL_LIGHT0, GL_AMBIENT, a0);
    glLightfv(GL_LIGHT1, GL_POSITION, p1);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, d1);

    GLfloat sp[] = {.35f, .35f, .35f, 1}, sh[] = {45};
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, sp);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, sh);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    glEnable(GL_COLOR_MATERIAL);
}

void drawLineToSpot(float x, float y, float z)
{
    glPushAttrib(GL_LIGHTING_BIT);

    glDisable(GL_LIGHTING);
    glColor3f(1.0f, 1.0f, 1.0f);

    glBegin(GL_LINES);
        glVertex3f(0.0f, height, 0.0f);
        glVertex3f(x, y, z);
    glEnd();

    glPopAttrib();
}


// DrawSpot
void drawSpot()
{
    float x = Tank::CYL_R * cos(theta);
    float z = Tank::CYL_R * sin(theta);
    float y = height;

    glPushMatrix();

        drawLineToSpot(x, y, z);

    glTranslatef(x, y, z);

    glDisable(GL_LIGHTING);
    glColor3f(1, 0, 0);

    glutSolidSphere(0.05, 16, 16);

    Transform tf;
    tf.set(0, 0, 0, 0, 90, -90);
    tf.apply();


 Transform tf_spot_perprndicular_to_tank;
    tf_spot_perprndicular_to_tank.set(0, 0, 0, 0, 0 ,(theta * 180.0f / M_PI));
    tf_spot_perprndicular_to_tank.apply();

    Axis("Spot", 1.0f).draw();

    glEnable(GL_LIGHTING);

    glPopMatrix();
}


// drawRobot
static void drawRobot()
{
    // glPushMatrix();

    Transform robotTf;

    robotTf.set(
        static_cast<float>(robotPose.x), // X
        static_cast<float>(robotPose.y), // Z
        0.0f,                       // Z

        0.0f,                                        // roll
        static_cast<float>(robotPose.yaw * 180.0 / M_PI), // pitch (Y axis)
        0.0f                                         // yaw
    );

    robotTf.apply();

    robot.draw();
}

// ── GLUT callbacks ───────────────────────────────────────────
static void display()
{
    glClearColor(0.07f, 0.08f, 0.10f, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, (double)W / H, 0.01, 200.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    float rT = camTheta * (float)M_PI / 180.f;
    float rP = camPhi * (float)M_PI / 180.f;
    gluLookAt(camDist * cosf(rP) * sinf(rT),
              camDist * sinf(rP),
              camDist * cosf(rP) * cosf(rT),
              0, 0, 0, 0, 1, 0);

    glEnable(GL_DEPTH_TEST);
    setupLighting();
    drawGrid();
    Axis("world", 1.0f).draw();

    drawTank();
    drawSpot();
    drawRobot();

    glutSwapBuffers();
}
static bool initJoystick()
{
    js_fd = open("/dev/input/js0", O_RDONLY | O_NONBLOCK);

    if (js_fd < 0)
    {
        std::cout << "Failed to open /dev/input/js0\n";
        return false;
    }

    lastTime = std::chrono::steady_clock::now();
    return true;
}
static void updatePoseFromJoystick()
{
    js_event event;

    while (read(js_fd, &event, sizeof(event)) > 0)
    {
        event.type &= ~JS_EVENT_INIT;

        if (event.type == JS_EVENT_AXIS)
        {
            if (event.number < 8)
            {
                axis[event.number] =
                    static_cast<float>(event.value) / 32767.0f;
            }
        }
    }

    auto now = std::chrono::steady_clock::now();

    double dt =
        std::chrono::duration<double>(
            now - lastTime)
            .count();

    lastTime = now;

    double linear_vel = -axis[1];
    double angular_vel = -axis[2];

    robotPose.x += linear_vel * std::cos(robotPose.yaw) * dt;
    theta -= (linear_vel / Tank::CYL_R) * dt;

    robotPose.y += linear_vel * std::sin(robotPose.yaw) * dt;
    height += angular_vel * dt;

    robotPose.yaw += angular_vel * dt;

    if (height < 0.0)
        height = 0.0;

    if (height > Tank::CYL_H)
        height = Tank::CYL_H;

        std::cout
        << "\rTheta: " << theta*57.2958
        << std::flush;
    // std::cout
    //     << "\rX: " << robotPose.x
    //     << "  Y: " << robotPose.y
    //     << "  Yaw: " << robotPose.yaw
    //     << "  V: " << linear_vel
    //     << "  W: " << angular_vel
    //     << "      "
    //     << std::flush;
}

static void idle()
{
    updatePoseFromJoystick();

    glutPostRedisplay();
}

static void keyboard(unsigned char k, int, int)
{
    const float SPD = 1.8f;
    switch (k)
    {
    case 'w':
    case 'W':
        robot.setSpeed(SPD, SPD);
        break;
    case 's':
    case 'S':
        robot.setSpeed(-SPD, -SPD);
        break;
    case 'a':
    case 'A':
        robot.setSpeed(SPD * 0.2f, SPD);
        break;
    case 'd':
    case 'D':
        robot.setSpeed(SPD, SPD * 0.2f);
        break;
    case 'q':
    case 'Q':
        robot.setSpeed(-SPD, SPD);
        break;
    case 'e':
    case 'E':
        robot.setSpeed(SPD, -SPD);
        break;
    case 'r':
    case 'R':
        robot.reset();
        camTheta = 40;
        camPhi = 20;
        camDist = 30;
        break;
    case 27:
        exit(0);
    }
}

static void keyUp(unsigned char k, int, int)
{
    switch (k)
    {
    case 'w':
    case 'W':
    case 's':
    case 'S':
    case 'a':
    case 'A':
    case 'd':
    case 'D':
    case 'q':
    case 'Q':
    case 'e':
    case 'E':
        robot.stop();
        break;
    }
}

static void mouse(int b, int s, int x, int y)
{
    if (b == GLUT_LEFT_BUTTON)
    {
        dragging = (s == GLUT_DOWN);
        lastX = x;
        lastY = y;
    }
    if (b == 3)
    {
        camDist -= 1.f;
        if (camDist < 3)
            camDist = 3;
    }
    if (b == 4)
    {
        camDist += 1.f;
        if (camDist > 120)
            camDist = 120;
    }
    glutPostRedisplay();
}

static void motion(int x, int y)
{
    if (dragging)
    {
        camTheta -= (x - lastX) * 0.5f;
        camPhi += (y - lastY) * 0.5f;
        if (camPhi > 89)
            camPhi = 89;
        if (camPhi < -89)
            camPhi = -89;
        lastX = x;
        lastY = y;
        glutPostRedisplay();
    }
}

static void reshape(int w, int h)
{
    W = w;
    H = h ? h : 1;
    glViewport(0, 0, W, H);
}

// ── entry point ──────────────────────────────────────────────
int main(int argc, char **argv)
{
    if (!initJoystick())
    {
        return 1;
    }

    glutInit(&argc, argv);
    quad = gluNewQuadric();

    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(W, H);
    glutCreateWindow("Magnetic Robot on Tank – Wheel / Robot / Tank classes");

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutKeyboardUpFunc(keyUp);
    glutMouseFunc(mouse);
    glutMotionFunc(motion);
    glutIdleFunc(idle);

    glutMainLoop();

    gluDeleteQuadric(quad);
    return 0;
}