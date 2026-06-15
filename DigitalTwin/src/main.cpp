#include <GL/glut.h>
#include <cmath>
#include <cstdio>

#include "core/Tank.cpp"   
#include "core/Robot.cpp" 

GLUquadric* quad = NULL;

// ── window ───────────────────────────────────────────────────
static int W = 1280, H = 800;

// ── scene objects ────────────────────────────────────────────
static Tank  tank;
static Robot robot;

// ── camera ───────────────────────────────────────────────────
static float camTheta = 90.f, camPhi = 20.f, camDist = 75.f;
static int   lastX = 0, lastY = 0;
static bool  dragging = false;

// ── world-frame axis arrows ──────────────────────────────────
static void drawArrow(float len)
{
    GLUquadric* q = gluNewQuadric();
    gluCylinder(q, 0.08f, 0.08f, len*0.8f, 8, 1);
    glPushMatrix();
      glTranslatef(0,0,len*0.8f);
      gluCylinder(q, 0.20f, 0.f, len*0.2f, 8, 1);
    glPopMatrix();
    gluDeleteQuadric(q);
}

void drawTank() {
    // Tank body (cylinder)
    glColor3f(0.5f, 0.5f, 0.6f);  // Steel color
    gluCylinder(quad, 2.0f, 2.0f, 4.0f, 32, 32); 

}
static void drawWorldAxes()
{
    glPushAttrib(GL_LIGHTING_BIT);
    glDisable(GL_LIGHTING);
    glColor3f(1.f,0.1f,0.1f);
    glPushMatrix(); glRotatef(90,0,1,0); drawArrow(5.f); glPopMatrix();
    glColor3f(0.1f,1.f,0.1f);
    glPushMatrix(); glRotatef(-90,1,0,0); drawArrow(5.f); glPopMatrix();
    glColor3f(0.2f,0.4f,1.f);
    drawArrow(5.f);
    glPopAttrib();
}

// ── HUD ──────────────────────────────────────────────────────
static void drawHUD()
{
    char buf[256];
    snprintf(buf, sizeof(buf),
        "W/S=fwd/back  A/D=turn  Q/E=spin  R=reset  "
        "| theta=%.1f  h=%.2f  phi=%.1f deg",
        robot.theta * 180.f/(float)M_PI,
        robot.height,
        robot.phi   * 180.f/(float)M_PI);

    glMatrixMode(GL_PROJECTION); glPushMatrix(); glLoadIdentity();
    gluOrtho2D(0, W, 0, H);
    glMatrixMode(GL_MODELVIEW);  glPushMatrix(); glLoadIdentity();
    glDisable(GL_LIGHTING);
    glColor3f(0.75f, 0.95f, 0.65f);
    glRasterPos2f(10, 10);
    for(char* p = buf; *p; p++) glutBitmapCharacter(GLUT_BITMAP_9_BY_15, *p);
    glEnable(GL_LIGHTING);
    glPopMatrix();
    glMatrixMode(GL_PROJECTION); glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}

// ── lighting setup ───────────────────────────────────────────
static void setupLighting()
{
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHT1);
    glEnable(GL_NORMALIZE);

    GLfloat p0[]={15,20,15,1}, d0[]={1,.95f,.85f,1}, a0[]={.22f,.22f,.22f,1};
    GLfloat p1[]={-10,10,-10,1}, d1[]={.3f,.4f,.6f,1};
    glLightfv(GL_LIGHT0,GL_POSITION,p0);
    glLightfv(GL_LIGHT0,GL_DIFFUSE, d0);
    glLightfv(GL_LIGHT0,GL_AMBIENT, a0);
    glLightfv(GL_LIGHT1,GL_POSITION,p1);
    glLightfv(GL_LIGHT1,GL_DIFFUSE, d1);

    GLfloat sp[]={.35f,.35f,.35f,1}, sh[]={45};
    glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR, sp);
    glMaterialfv(GL_FRONT_AND_BACK,GL_SHININESS,sh);
    glColorMaterial(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE);
    glEnable(GL_COLOR_MATERIAL);
}

// ── GLUT callbacks ───────────────────────────────────────────
static void display()
{
    glClearColor(0.07f,0.08f,0.10f,1);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

    drawTank();
    glutSwapBuffers();
}

static void idle()
{
    robot.update(Tank::CYL_R, Tank::CYL_H);
    glutPostRedisplay();
}

static void keyboard(unsigned char k, int, int)
{
    const float SPD = 1.8f;
    switch(k){
    case 'w': case 'W': robot.setSpeed( SPD,        SPD       ); break;
    case 's': case 'S': robot.setSpeed(-SPD,       -SPD       ); break;
    case 'a': case 'A': robot.setSpeed( SPD*0.2f,   SPD       ); break;
    case 'd': case 'D': robot.setSpeed( SPD,        SPD*0.2f  ); break;
    case 'q': case 'Q': robot.setSpeed(-SPD,        SPD       ); break;
    case 'e': case 'E': robot.setSpeed( SPD,       -SPD       ); break;
    case 'r': case 'R':
        robot.reset();
        camTheta=40; camPhi=20; camDist=30;
        break;
    case 27: exit(0);
    }
}

static void keyUp(unsigned char k, int, int)
{
    switch(k){
    case 'w': case 'W': case 's': case 'S':
    case 'a': case 'A': case 'd': case 'D':
    case 'q': case 'Q': case 'e': case 'E':
        robot.stop(); break;
    }
}

static void mouse(int b, int s, int x, int y)
{
    if(b==GLUT_LEFT_BUTTON){ dragging=(s==GLUT_DOWN); lastX=x; lastY=y; }
    if(b==3){ camDist-=1.f; if(camDist<3)  camDist=3;  }
    if(b==4){ camDist+=1.f; if(camDist>120)camDist=120; }
    glutPostRedisplay();
}

static void motion(int x, int y)
{
    if(dragging){
        camTheta -= (x-lastX)*0.5f;
        camPhi   += (y-lastY)*0.5f;
        if(camPhi> 89) camPhi= 89;
        if(camPhi<-89) camPhi=-89;
        lastX=x; lastY=y;
        glutPostRedisplay();
    }
}

static void reshape(int w, int h){ W=w; H=h?h:1; glViewport(0,0,W,H); }

// ── entry point ──────────────────────────────────────────────
int main(int argc, char** argv)
{
    glutInit(&argc,argv);
        quad = gluNewQuadric();

    glutInitDisplayMode(GLUT_DOUBLE|GLUT_RGB|GLUT_DEPTH);
    glutInitWindowSize(W,H);
    glutCreateWindow("Magnetic Robot on Tank – Wheel / Robot / Tank classes");

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutKeyboardUpFunc(keyUp);
    glutMouseFunc(mouse);
    glutMotionFunc(motion);
    glutIdleFunc(idle);
    gluDeleteQuadric(quad);

    glutMainLoop();
    return 0;
}