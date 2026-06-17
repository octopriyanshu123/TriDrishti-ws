#include <GL/glut.h>
#include <cmath>
#include <cstdio>

#include "core/FlatRobot.cpp"
#include <linux/joystick.h>
#include <fcntl.h>
#include <unistd.h>
#include <chrono>
#include <iostream>

int count = 0;
struct Pose
{
    double x = 0.0;
    double y = 0.0;
    double yaw = 0.0;
};

static Pose pose;

static int js_fd = -1;
static float axis[8] = {0};

static std::chrono::steady_clock::time_point lastTime;
// #include "core/Axis.cpp" // includes Wheel.cpp + Base.cpp internally

// ── window ───────────────────────────────────────────────────
static int W = 1280, H = 800;

// ── scene ────────────────────────────────────────────────────
static FlatRobot robot;

// ── camera ───────────────────────────────────────────────────
static float camTheta = 40.f, camPhi = 28.f, camDist = 8.f;
static int lastX = 0, lastY = 0;
static bool dragging = false;

// ── world axes ───────────────────────────────────────────────
static void drawArrow(float len)
{
    GLUquadric *q = gluNewQuadric();
    gluCylinder(q, len * .04f, len * .04f, len * .78f, 8, 1);
    glPushMatrix();
    glTranslatef(0, 0, len * .78f);
    gluCylinder(q, len * .10f, 0.f, len * .22f, 8, 1);
    glPopMatrix();
    gluDeleteQuadric(q);
}
// drawWorldAxes replaced by Axis("world", 1.0f).draw() in display()

// ── floor grid ───────────────────────────────────────────────
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

// ── lighting ─────────────────────────────────────────────────
static void setupLighting()
{
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHT1);
    glEnable(GL_NORMALIZE);
    GLfloat p0[] = {5, 8, 5, 1}, d0[] = {1, .95f, .85f, 1}, a0[] = {.22f, .22f, .22f, 1};
    GLfloat p1[] = {-4, 6, -4, 1}, d1[] = {.3f, .4f, .6f, 1};
    glLightfv(GL_LIGHT0, GL_POSITION, p0);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, d0);
    glLightfv(GL_LIGHT0, GL_AMBIENT, a0);
    glLightfv(GL_LIGHT1, GL_POSITION, p1);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, d1);
    GLfloat sp[] = {.30f, .30f, .30f, 1}, sh[] = {40};
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, sp);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, sh);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    glEnable(GL_COLOR_MATERIAL);
}

// ── HUD ──────────────────────────────────────────────────────
static void drawHUD()
{
    char buf[200];
    snprintf(buf, sizeof(buf),
             "W/S=fwd/back  A/D=turn  Q/E=spin  R=reset  "
             "| x=%.2f  z=%.2f  yaw=%.1f deg");
    //  robot.x(), robot.z(), yaw() * 180.f / (float)M_PI);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, W, 0, H);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glDisable(GL_LIGHTING);
    glColor3f(0.75f, 0.95f, 0.65f);
    glRasterPos2f(10, 10);
    for (char *p = buf; *p; p++)
        glutBitmapCharacter(GLUT_BITMAP_9_BY_15, *p);
    glEnable(GL_LIGHTING);
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}

// ── display ──────────────────────────────────────────────────
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

    // ── world origin axis ────────────────────────────────────
    Axis("world", 1.0f).draw();

    // ── floor grid ───────────────────────────────────────────
    drawGrid();

 

        
    glPushMatrix();

    Transform robotTf;

    robotTf.set(
        static_cast<float>(pose.x), // X
        static_cast<float>(pose.y), // Z
        0.0f, // Z

        0.0f,                                        // roll
        static_cast<float>(pose.yaw * 180.0 / M_PI), // pitch (Y axis)
        0.0f                                         // yaw
    );

    robotTf.apply();

    robot.draw();

    glPopMatrix();

    drawHUD();
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

    pose.x += linear_vel *
              std::cos(pose.yaw) * dt;

    pose.y += linear_vel *
              std::sin(pose.yaw) * dt;

    pose.yaw += angular_vel * dt;

    std::cout
        << count++
        << "\rX: " << pose.x
        << "  Y: " << pose.y
        << "  Yaw: " << pose.yaw
        << "  V: " << linear_vel
        << "  W: " << angular_vel
        << "      "
        << std::flush;
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
        robot.setSpeed(SPD * .2f, SPD);
        break;
    case 'd':
    case 'D':
        robot.setSpeed(SPD, SPD * .2f);
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
        camPhi = 28;
        camDist = 8;
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
        camDist -= .3f;
        if (camDist < .5f)
            camDist = .5f;
    }
    if (b == 4)
    {
        camDist += .3f;
        if (camDist > 40.f)
            camDist = 40.f;
    }
    glutPostRedisplay();
}
static void motion(int x, int y)
{
    if (dragging)
    {
        camTheta -= (x - lastX) * .5f;
        camPhi += (y - lastY) * .5f;
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

int main(int argc, char **argv)
{
    if (!initJoystick())
    {
        return 1;
    }

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(W, H);
    glutCreateWindow("Flat Ground Robot – Base + Wheel classes");

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
    return 0;
}
