// ============================================================
//  Wheel.cpp
//  One magnetic wheel: TYRE (outer, contacts tank) +
//                      RIM  (inner hub, floats clear)
//
//  Geometry (real mm / 200 scale):
//    TYRE_R = 0.3875  (77.5 mm)
//    TYRE_T = 0.20    (40 mm)
//    RIM_R  = 0.30
//    RIM_T  = 0.10
//
//  The wheel can ONLY spin around its own Z axis.
//  Spin is driven externally by Robot via setSpeed().
// ============================================================
#include <GL/glut.h>
#include <cmath>
#include "Axis.cpp"

class Wheel
{
public:
    // ── geometry constants ───────────────────────────────────
    static constexpr float TYRE_R = 0.3875f;
    static constexpr float TYRE_T = 0.20f;
    static constexpr float RIM_R = 0.30f;
    static constexpr float RIM_T = 0.10f;
    static constexpr int SEG = 48;

    // ── state ────────────────────────────────────────────────
    float spinDeg = 0.f;
    float speedDeg = 0.f;

    // FIX 1 – plain instance member (not static), with default
    bool isRight = false;

    // FIX 2 – constructor after all member declarations
    Wheel() = default;
    explicit Wheel(bool right) : isRight(right) {}

    // ── interface ────────────────────────────────────────────
    void setSpeed(float deg_per_frame) { speedDeg = deg_per_frame; }
    void stop() { speedDeg = 0.f; }
    void reset() { spinDeg = speedDeg = 0.f; }

    void update() { spinDeg += speedDeg; }

    void draw(int isRight) const
    {
        glPushMatrix();
        glRotatef(spinDeg, 0, 0, 1);

        glColor3f(0.14f, 0.14f, 0.14f);
        drawTyre();

        glColor3f(0.42f, 0.42f, 0.45f);
        drawRim();

        drawSpokes();

        // drawAxes(TYRE_R * 1.8f, isRight);
        Axis a1("base_link", 0.3f);
        a1.setOrientation(90.f, 0.f, 0.f); // rotate so X (outward) → Y (up), Y (up) → -X (backward)
        a1.draw();

        glPopMatrix();
    }

private:
    static void drawCylZ(float r, float halfT, int seg)
    {
        glBegin(GL_TRIANGLE_STRIP);
        for (int i = 0; i <= seg; i++)
        {
            float a = (float)i / seg * 2.f * (float)M_PI;
            float c = cosf(a), s = sinf(a);
            glNormal3f(c, s, 0);
            glVertex3f(r * c, r * s, -halfT);
            glVertex3f(r * c, r * s, halfT);
        }
        glEnd();
        for (int side = -1; side <= 1; side += 2)
        {
            float zz = (float)side * halfT;
            glBegin(GL_TRIANGLE_FAN);
            glNormal3f(0, 0, (float)side);
            glVertex3f(0, 0, zz);
            for (int i = 0; i <= seg; i++)
            {
                float a = (float)(side * i) / seg * 2.f * (float)M_PI;
                glVertex3f(r * cosf(a), r * sinf(a), zz);
            }
            glEnd();
        }
    }

    void drawTyre() const
    {
        float half = TYRE_T * 0.5f;
        glBegin(GL_TRIANGLE_STRIP);
        for (int i = 0; i <= SEG; i++)
        {
            float a = (float)i / SEG * 2.f * (float)M_PI;
            float c = cosf(a), s = sinf(a);
            glNormal3f(c, s, 0);
            glVertex3f(TYRE_R * c, TYRE_R * s, -half);
            glVertex3f(TYRE_R * c, TYRE_R * s, half);
        }
        glEnd();
        glBegin(GL_TRIANGLE_STRIP);
        for (int i = 0; i <= SEG; i++)
        {
            float a = (float)i / SEG * 2.f * (float)M_PI;
            float c = cosf(a), s = sinf(a);
            glNormal3f(-c, -s, 0);
            glVertex3f(RIM_R * c, RIM_R * s, half);
            glVertex3f(RIM_R * c, RIM_R * s, -half);
        }
        glEnd();
        for (int side = -1; side <= 1; side += 2)
        {
            float zz = (float)side * half;
            glBegin(GL_TRIANGLE_STRIP);
            glNormal3f(0, 0, (float)side);
            for (int i = 0; i <= SEG; i++)
            {
                float a = (float)(side * i) / SEG * 2.f * (float)M_PI;
                float c = cosf(a), s = sinf(a);
                glVertex3f(TYRE_R * c, TYRE_R * s, zz);
                glVertex3f(RIM_R * c, RIM_R * s, zz);
            }
            glEnd();
        }
    }

    void drawRim() const { drawCylZ(RIM_R, RIM_T * 0.5f, SEG); }

    void drawSpokes() const
    {
        glPushAttrib(GL_LIGHTING_BIT);
        glDisable(GL_LIGHTING);
        glColor3f(0.70f, 0.70f, 0.72f);
        glLineWidth(2.f);
        float zz = TYRE_T * 0.5f + 0.002f;
        glBegin(GL_LINES);
        for (int sp = 0; sp < 3; sp++)
        {
            float a = (float)sp / 3.f * 2.f * (float)M_PI;
            float c = cosf(a) * RIM_R, s = sinf(a) * RIM_R;
            glVertex3f(0, 0, zz);
            glVertex3f(c, s, zz);
            glVertex3f(0, 0, -zz);
            glVertex3f(c, s, -zz);
        }
        glEnd();
        glPopAttrib();
    }

    static void drawArrow(float len)
    {
        GLUquadric *q = gluNewQuadric();
        gluCylinder(q, len * 0.06f, len * 0.06f, len * 0.75f, 8, 1);
        glPushMatrix();
        glTranslatef(0, 0, len * 0.75f);
        gluCylinder(q, len * 0.14f, 0.f, len * 0.25f, 8, 1);
        glPopMatrix();
        gluDeleteQuadric(q);
    }

    void drawAxes(float scale, int isRight) const
    {
        glPushAttrib(GL_LIGHTING_BIT);
        glDisable(GL_LIGHTING);

        // X – red
        glColor3f(1.f, 0.15f, 0.15f);
        glPushMatrix();
        glRotatef(90, 0, 1, 0);
        drawArrow(scale);
        glPopMatrix();

        // Y – green
        glColor3f(0.15f, 1.f, 0.15f);
        glPushMatrix();
        glRotatef(-90, 1, 0, 0);
        drawArrow(scale);
        glPopMatrix();

        // Z – blue  (axle / spin axis)
        glColor3f(0.2f, 0.45f, 1.f);
        glPushMatrix();

        if (isRight == 1)
        {
            glRotatef(180.f, 0, 1, 0);
        }
        else
        {
            glRotatef(0.f, 0, 1, 0);
        }
        drawArrow(scale);
        glPopMatrix();
        glPopAttrib();
    }
};