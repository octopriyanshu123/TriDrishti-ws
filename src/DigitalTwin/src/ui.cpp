#include <GL/glut.h>
#include <cmath>
#include <cstdio>
#include <cstring>

#include "sensor/lidar.cpp"

// ─────────────────────────────────────────────
//  Globals
// ─────────────────────────────────────────────
static Lidar2D lidar;

static const float WORLD_HALF = 5.0f;
static const float MOVE_STEP  = 0.1f;
static const float ROT_STEP   = 5.0f;

// GLUT window IDs
static int winWorld  = 0;   // Window 1: world / simulation
static int winOutput = 0;   // Window 2: LiDAR output

static int worldW  = 800, worldH  = 800;
static int outputW = 450, outputH = 500;

// ─────────────────────────────────────────────
//  Shared input handling
// ─────────────────────────────────────────────
static void handleKey(unsigned char key)
{
    float headRad = lidar.headingDeg * (3.14159265f / 180.0f);
    float fx = std::cos(headRad), fy = std::sin(headRad);
    float rx =  fy, ry = -fx;

    switch (key) {
        case 'w': case 'W':
            lidar.posX += fx * MOVE_STEP; lidar.posY += fy * MOVE_STEP; break;
        case 's': case 'S':
            lidar.posX -= fx * MOVE_STEP; lidar.posY -= fy * MOVE_STEP; break;
        case 'a': case 'A':
            lidar.posX -= rx * MOVE_STEP; lidar.posY -= ry * MOVE_STEP; break;
        case 'd': case 'D':
            lidar.posX += rx * MOVE_STEP; lidar.posY += ry * MOVE_STEP; break;
        case 'q': case 'Q': lidar.headingDeg += ROT_STEP; break;
        case 'e': case 'E': lidar.headingDeg -= ROT_STEP; break;
        case 'r': case 'R':
            lidar.setPosition(0.0f, 0.0f);
            lidar.setHeading(0.0f);
            break;
        case 27: exit(0);
    }
    glutSetWindow(winWorld);  glutPostRedisplay();
    glutSetWindow(winOutput); glutPostRedisplay();
}

static void handleSpecial(int key)
{
    switch (key) {
        case GLUT_KEY_UP:    lidar.posY += MOVE_STEP; break;
        case GLUT_KEY_DOWN:  lidar.posY -= MOVE_STEP; break;
        case GLUT_KEY_LEFT:  lidar.posX -= MOVE_STEP; break;
        case GLUT_KEY_RIGHT: lidar.posX += MOVE_STEP; break;
    }
    glutSetWindow(winWorld);  glutPostRedisplay();
    glutSetWindow(winOutput); glutPostRedisplay();
}

// ─────────────────────────────────────────────
//  Pixel-space text helper
// ─────────────────────────────────────────────
static void drawText2D(float px, float py, const char* str, int winW, int winH)
{
    glMatrixMode(GL_PROJECTION);
    glPushMatrix(); glLoadIdentity();
    gluOrtho2D(0, winW, 0, winH);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix(); glLoadIdentity();

    glRasterPos2f(px, py);
    for (const char* c = str; *c; ++c)
        glutBitmapCharacter(GLUT_BITMAP_8_BY_13, *c);

    glPopMatrix();
    glMatrixMode(GL_PROJECTION); glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}

// ═════════════════════════════════════════════
//  WINDOW 1 — World view
// ═════════════════════════════════════════════
static void displayWorld()
{
    glutSetWindow(winWorld);
    glClear(GL_COLOR_BUFFER_BIT);

    glMatrixMode(GL_PROJECTION); glLoadIdentity();
    gluOrtho2D(-WORLD_HALF, WORLD_HALF, -WORLD_HALF, WORLD_HALF);
    glMatrixMode(GL_MODELVIEW); glLoadIdentity();

    // Grid
    glLineWidth(1.0f);
    glColor4f(0.18f, 0.18f, 0.22f, 1.0f);
    glBegin(GL_LINES);
    for (int i = -(int)WORLD_HALF; i <= (int)WORLD_HALF; ++i) {
        glVertex2f((float)i, -WORLD_HALF); glVertex2f((float)i,  WORLD_HALF);
        glVertex2f(-WORLD_HALF, (float)i); glVertex2f( WORLD_HALF, (float)i);
    }
    glEnd();

    // Axes
    glLineWidth(1.5f);
    glColor3f(0.45f, 0.1f, 0.1f);
    glBegin(GL_LINES);
        glVertex2f(-WORLD_HALF, 0); glVertex2f(WORLD_HALF, 0);
    glEnd();
    glColor3f(0.1f, 0.45f, 0.1f);
    glBegin(GL_LINES);
        glVertex2f(0, -WORLD_HALF); glVertex2f(0, WORLD_HALF);
    glEnd();

    lidar.scan();
    lidar.draw();

    // HUD
    char buf[160];
    glColor3f(0.65f, 0.65f, 0.65f);
    snprintf(buf, sizeof(buf), "Pos: (%.2f, %.2f)  Heading: %.0f deg",
             lidar.posX, lidar.posY, lidar.headingDeg);
    drawText2D(10, worldH - 18, buf, worldW, worldH);

    snprintf(buf, sizeof(buf),
             "Range: %.1f u   Res: %.1f deg/ray   Rays: %d",
             LIDAR_RANGE, ANGLE_RESOLUTION, NUM_RAYS);
    drawText2D(10, worldH - 34, buf, worldW, worldH);

    glColor3f(0.45f, 0.45f, 0.45f);
    drawText2D(10, 8,
        "WASD/Arrows: move   Q/E: rotate   R: reset   ESC: quit",
        worldW, worldH);

    glutSwapBuffers();
}

static void reshapeWorld(int w, int h)
{
    worldW = w; worldH = h;
    glViewport(0, 0, w, h);
}

static void keyboardWorld(unsigned char key, int, int) { handleKey(key); }
static void specialWorld(int key, int, int)            { handleSpecial(key); }

//  WINDOW 2 — LiDAR output view

static void drawPolarPlot(int x0, int y0, int w, int h,
                           const std::vector<Hit>& hits)
{
    
    float cx = x0 + w * 0.5f;
    float cy = y0 + h * 0.5f;
    float R  = (w < h ? w : h) * 0.44f;   // max pixel radius

    // ── concentric range rings ──
    glLineWidth(1.0f);
    for (int ring = 1; ring <= 4; ++ring) {
        float r = R * ring / 4.0f;
        if (ring == 4) glColor3f(0.25f, 0.55f, 0.25f);
        else           glColor3f(0.18f, 0.30f, 0.18f);
        glBegin(GL_LINE_LOOP);
        for (int a = 0; a < 360; ++a) {
            float ar = a * (3.14159265f / 180.0f);
            glVertex2f(cx + r * std::cos(ar), cy + r * std::sin(ar));
        }
        glEnd();
    }

    // ── radial spokes every 45° ──
    glColor3f(0.18f, 0.28f, 0.18f);
    for (int a = 0; a < 360; a += 45) {
        float ar = a * (3.14159265f / 180.0f);
        glBegin(GL_LINES);
            glVertex2f(cx, cy);
            glVertex2f(cx + R * std::cos(ar), cy + R * std::sin(ar));
        glEnd();
    }

    // ── ring labels ──
    glColor3f(0.35f, 0.55f, 0.35f);
    char lbuf[16];
    for (int ring = 1; ring <= 4; ++ring) {
        float r = R * ring / 4.0f;
        snprintf(lbuf, sizeof(lbuf), "%.1fu", LIDAR_RANGE * ring / 4.0f);
        glRasterPos2f(cx + r + 3, cy + 3);
        for (const char* c = lbuf; *c; ++c)
            glutBitmapCharacter(GLUT_BITMAP_8_BY_13, *c);
    }

    // ── degree labels ──
    glColor3f(0.30f, 0.50f, 0.30f);
    const char* degLabels[] = {"0","45","90","135","180","225","270","315"};
    for (int i = 0; i < 8; ++i) {
        float ar = i * 45.0f * (3.14159265f / 180.0f);
        float lx = cx + (R + 14) * std::cos(ar);
        float ly = cy + (R + 14) * std::sin(ar);
        glRasterPos2f(lx - 6, ly - 4);
        for (const char* c = degLabels[i]; *c; ++c)
            glutBitmapCharacter(GLUT_BITMAP_8_BY_13, *c);
    }

    // ── hit point cloud ──
    // Draw filled polygon connecting all hit points first (area sweep)
    glColor4f(0.0f, 0.55f, 0.15f, 0.12f);
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(cx, cy);
    for (int i = 0; i < NUM_RAYS; ++i) {
        const Hit& h = hits[i];
        float ar = (lidar.headingDeg + i * ANGLE_RESOLUTION) * (3.14159265f / 180.0f);
        float d  = h.dist / LIDAR_RANGE;
        glVertex2f(cx + d * R * std::cos(ar), cy + d * R * std::sin(ar));
    }
    // close the fan
    {
        const Hit& h = hits[0];
        float ar = lidar.headingDeg * (3.14159265f / 180.0f);
        float d  = h.dist / LIDAR_RANGE;
        glVertex2f(cx + d * R * std::cos(ar), cy + d * R * std::sin(ar));
    }
    glEnd();

    // Outline of the sweep shape
    glLineWidth(1.2f);
    glColor4f(0.0f, 0.85f, 0.25f, 0.45f);
    glBegin(GL_LINE_LOOP);
    for (int i = 0; i < NUM_RAYS; ++i) {
        const Hit& h = hits[i];
        float ar = (lidar.headingDeg + i * ANGLE_RESOLUTION) * (3.14159265f / 180.0f);
        float d  = h.dist / LIDAR_RANGE;
        glVertex2f(cx + d * R * std::cos(ar), cy + d * R * std::sin(ar));
    }
    glEnd();

    // Individual hit dots
    glPointSize(2.5f);
    glBegin(GL_POINTS);
    for (int i = 0; i < NUM_RAYS; ++i) {
        const Hit& h = hits[i];
        if (!h.valid) continue;
        float ar = (lidar.headingDeg + i * ANGLE_RESOLUTION) * (3.14159265f / 180.0f);
        float d  = h.dist / LIDAR_RANGE;
        float bright = 0.5f + 0.5f * (1.0f - d);
        glColor3f(0.0f, bright, 0.3f * bright);
        glVertex2f(cx + d * R * std::cos(ar), cy + d * R * std::sin(ar));
    }
    glEnd();

    // Sensor origin dot
    glPointSize(6.0f);
    glColor3f(1.0f, 0.8f, 0.0f);
    glBegin(GL_POINTS); glVertex2f(cx, cy); glEnd();

    // Heading arrow
    float har = lidar.headingDeg * (3.14159265f / 180.0f);
    glLineWidth(2.0f);
    glColor3f(1.0f, 0.6f, 0.0f);
    glBegin(GL_LINES);
        glVertex2f(cx, cy);
        glVertex2f(cx + 0.12f * R * std::cos(har),
                   cy + 0.12f * R * std::sin(har));
    glEnd();

    // Panel title
    glColor3f(0.55f, 0.55f, 0.55f);
    glRasterPos2f(x0 + 6, y0 + h - 16);
    const char* title = "Polar View (sensor frame)";
    for (const char* c = title; *c; ++c)
        glutBitmapCharacter(GLUT_BITMAP_8_BY_13, *c);
}

static void displayOutput()
{
    glutSetWindow(winOutput);
    glClear(GL_COLOR_BUFFER_BIT);

    // Full pixel-space ortho
    glMatrixMode(GL_PROJECTION); glLoadIdentity();
    gluOrtho2D(0, outputW, 0, outputH);
    glMatrixMode(GL_MODELVIEW); glLoadIdentity();

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    const std::vector<Hit>& hits = lidar.hits;

    // Divider
    glColor3f(0.22f, 0.22f, 0.25f);
    glLineWidth(1.0f);
    glBegin(GL_LINES);
        glVertex2f(outputW / 2, 0);
        glVertex2f(outputW / 2, outputH);
    glEnd();



    drawPolarPlot(0,     0, outputW,          outputH, hits);



    glutSwapBuffers();
}

static void reshapeOutput(int w, int h)
{
    outputW = w; outputH = h;
    glViewport(0, 0, w, h);
}

static void keyboardOutput(unsigned char key, int, int) { handleKey(key); }
static void specialOutput(int key, int, int)            { handleSpecial(key); }


static void timer(int)
{
    glutSetWindow(winWorld);  glutPostRedisplay();
    glutSetWindow(winOutput); glutPostRedisplay();
    glutTimerFunc(16, timer, 0);
}

int main(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);

    // ── Window 1: World ──
    glutInitWindowSize(worldW, worldH);
    glutInitWindowPosition(80, 80);
    winWorld = glutCreateWindow("2D LiDAR — World View");

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glClearColor(0.05f, 0.05f, 0.08f, 1.0f);

    glutDisplayFunc(displayWorld);
    glutReshapeFunc(reshapeWorld);
    glutKeyboardFunc(keyboardWorld);
    glutSpecialFunc(specialWorld);

    // ── Window 2: LiDAR output ──
    glutInitWindowSize(outputW, outputH);
    glutInitWindowPosition(900, 80);
    winOutput = glutCreateWindow("2D LiDAR — Output View");

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glClearColor(0.05f, 0.05f, 0.08f, 1.0f);

    glutDisplayFunc(displayOutput);
    glutReshapeFunc(reshapeOutput);
    glutKeyboardFunc(keyboardOutput);
    glutSpecialFunc(specialOutput);

    // ── Shared setup ──
    lidar.init(0.0f, 0.0f, 0.0f);
    // lidar.scan();  

    glutTimerFunc(16, timer, 0);
    glutMainLoop();
    return 0;
}