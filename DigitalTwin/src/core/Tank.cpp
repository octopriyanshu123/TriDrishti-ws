// ============================================================
//  Tank.cpp
//  Metallic oil/chemical tank – hollow cylinder.
//  The robot's magnetic wheels grip the OUTER surface.
//
//  Geometry:
//    CYL_R  = 10.0   outer radius
//    CYL_H  = 50.0   total height
//    CYL_SEG = 72    circumference segments
//
//  Coordinate frame (tank TF = world):
//    Origin at base centre
//    Y  = up
//    XZ = ground plane
// ============================================================
#include <GL/glut.h>
#include <cmath>

class Tank
{
public:
    // ── geometry constants ───────────────────────────────────
    static constexpr float CYL_R  = 10.0f;
    static constexpr float CYL_H  = 20.00f;
    static constexpr int   SEG    = 72;
    static constexpr int   LAT    = 8;    // latitude  grid rings
    static constexpr int   LON    = 16;   // longitude grid lines

    // draw the full tank (outer wall + caps + surface grid)
    void draw() const
    {
        float yB = 0.0f; // base at 0,0,0
        float yT = CYL_H; // Hight 

        drawOuterWall(yB, yT);
        drawCaps(yB, yT);
        drawSurfaceGrid(yB, yT);
    }

    // draw world-floor grid below the tank
    void drawGrid() const
    {
        float y = -CYL_H*0.5f - 0.6f;
        glPushAttrib(GL_LIGHTING_BIT);
        glDisable(GL_LIGHTING);
        glBegin(GL_LINES);
        for(int i = -20; i <= 20; i++){
            float f = (float)i * 2.f;
            bool maj = (i % 4 == 0);
            if(maj) glColor4f(.50f,.50f,.52f,.8f);
            else    glColor4f(.28f,.28f,.30f,.4f);
            glVertex3f(f, y,-40); glVertex3f(f, y, 40);
            glVertex3f(-40, y, f); glVertex3f(40, y, f);
        }
        glEnd();
        glPopAttrib();
    }

private:
    void drawOuterWall(float yB, float yT) const
    {
        glColor3f(0.20f, 0.26f, 0.36f);
        glBegin(GL_TRIANGLE_STRIP);
        for(int i = 0; i <= SEG; i++){
            float a = (float)i/SEG * 2.f*(float)M_PI;
            float c = cosf(a), s = sinf(a);
            glNormal3f(c, 0, s);
            glVertex3f(CYL_R*c, yB, CYL_R*s);
            glVertex3f(CYL_R*c, yT, CYL_R*s);
        }
        glEnd();
    }

    void drawCaps(float yB, float yT) const
    {
        glColor3f(0.15f, 0.18f, 0.26f);
        for(int cap = 0; cap < 2; cap++){
            float yy = (cap==0) ? yB : yT;
            float ny = (cap==0) ? -1.f : 1.f;
            glBegin(GL_TRIANGLE_FAN);
            glNormal3f(0, ny, 0);
            glVertex3f(0, yy, 0);
            for(int i = 0; i <= SEG; i++){
                float a = (float)(cap==0 ? -i : i)/SEG * 2.f*(float)M_PI;
                glVertex3f(CYL_R*cosf(a), yy, CYL_R*sinf(a));
            }
            glEnd();
        }
    }

    void drawSurfaceGrid(float yB, float yT) const
    {
        glPushAttrib(GL_LIGHTING_BIT);
        glDisable(GL_LIGHTING);
        glColor3f(0.34f, 0.40f, 0.56f);
        glLineWidth(1.f);

        // latitude rings
        for(int j = 0; j <= LAT; j++){
            float yy = yB + (float)j/LAT * CYL_H;
            glBegin(GL_LINE_LOOP);
            for(int i = 0; i <= SEG; i++){
                float a = (float)i/SEG * 2.f*(float)M_PI;
                glVertex3f((CYL_R+.01f)*cosf(a), yy, (CYL_R+.01f)*sinf(a));
            }
            glEnd();
        }

        // longitude lines
        for(int i = 0; i < LON; i++){
            float a = (float)i/LON * 2.f*(float)M_PI;
            float c = cosf(a), s = sinf(a);
            glBegin(GL_LINES);
            glVertex3f((CYL_R+.01f)*c, yB, (CYL_R+.01f)*s);
            glVertex3f((CYL_R+.01f)*c, yT, (CYL_R+.01f)*s);
            glEnd();
        }
        glPopAttrib();
    }
};