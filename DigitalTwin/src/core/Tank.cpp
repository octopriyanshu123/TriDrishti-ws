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
    static constexpr float CYL_R = 10.0f;
    static constexpr float CYL_H = 20.00f;
    static constexpr int SEG = 72;
    static constexpr int LAT = 8;  // latitude  grid rings
    static constexpr int LON = 16; // longitude grid lines

    // draw the full tank (outer wall + caps + surface grid)
    void draw() const
    {
        float yB = 0.0f;  // base at 0,0,0
        float yT = CYL_H; // Hight

        drawOuterWall(yB, yT);
        drawCaps(yB, yT);
        drawSurfaceGrid(yB, yT);
       drawStair();

        drawSprayNozzles(5);
                drawSprayNozzles(8);


        //drawBottomManhole();
    }

    // draw world-floor grid below the tank
    void drawGrid() const
    {
        float y = -CYL_H * 0.5f - 0.6f;
        glPushAttrib(GL_LIGHTING_BIT);
        glDisable(GL_LIGHTING);
        glBegin(GL_LINES);
        for (int i = -20; i <= 20; i++)
        {
            float f = (float)i * 2.f;
            bool maj = (i % 4 == 0);
            if (maj)
                glColor4f(.50f, .50f, .52f, .8f);
            else
                glColor4f(.28f, .28f, .30f, .4f);
            glVertex3f(f, y, -40);
            glVertex3f(f, y, 40);
            glVertex3f(-40, y, f);
            glVertex3f(40, y, f);
        }
        glEnd();
        glPopAttrib();
    }

private:
void drawSprayNozzles(float hightFromTop) const
{
    const float ringR = CYL_R + 0.18f;
    const float y     = CYL_H - hightFromTop;

    //====================================================
    // Fire Water Ring Main (Red Circular Pipe)
    //====================================================

    glColor3f(0.85f,0.05f,0.05f);

    glLineWidth(5.0f);

    glBegin(GL_LINE_LOOP);

    for(int i=0;i<72;i++)
    {
        float a = i * 2.0f * M_PI / 72.0f;

        glVertex3f(
            ringR*cosf(a),
            y,
            ringR*sinf(a)
        );
    }

    glEnd();


    //====================================================
    // Nozzles
    //====================================================

    glColor3f(0.80f,0.80f,0.80f);

    for(int i=0;i<24;i++)
    {
        float a = i * 2.0f * M_PI / 24.0f;

        float x = ringR*cosf(a);
        float z = ringR*sinf(a);

        //------------------------------------------------
        // Branch Pipe
        //------------------------------------------------

        glBegin(GL_LINES);

        glVertex3f(x,y,z);

        glVertex3f(
            (ringR-0.18f)*cosf(a),
            y-0.15f,
            (ringR-0.18f)*sinf(a)
        );

        glEnd();

        //------------------------------------------------
        // Nozzle
        //------------------------------------------------

        glPushMatrix();

        glTranslatef(
            (ringR-0.18f)*cosf(a),
            y-0.15f,
            (ringR-0.18f)*sinf(a));

        glRotatef(-30,1,0,0);
        glRotatef(-a*180.0f/M_PI+90,0,1,0);

        GLUquadric* q = gluNewQuadric();

        gluCylinder(q,
                    0.05,
                    0.03,
                    0.35,
                    8,
                    1);

        gluDeleteQuadric(q);

        glPopMatrix();
    }
}

void drawStair() const
{
    const float stairR = CYL_R + 0.65f;

    const int steps = 500.0;
    const float turns = 1.0f;

    const float treadWidth = 0.65f;
    const float treadDepth = 0.20f;
    const float treadHeight = 0.04f;

    const float railHeight = 0.90f;



    glColor3f(0.55f,0.55f,0.55f);

    for(int i=0;i<steps;i++)
    {
        float t=i/(float)(steps-1);

        float ang=t*turns*2.0f*M_PI;

        float x=stairR*cosf(ang);
        float y=t*CYL_H;
        float z=stairR*sinf(ang);

        glPushMatrix();

        glTranslatef(x,y,z);

        glRotatef(-ang*180.0f/M_PI+90,0,1,0);

        glScalef(treadDepth,
                 treadHeight,
                 treadWidth);

        glutSolidCube(1.0);

        glPopMatrix();
    }


    glColor3f(1.0f,0.85f,0.0f);

    glLineWidth(3);

    glBegin(GL_LINE_STRIP);

    for(int i=0;i<=steps;i++)
    {
        float t=i/(float)steps;

        float ang=t*turns*2.0f*M_PI;

        glVertex3f(
            (stairR+0.32f)*cosf(ang),
            t*CYL_H+railHeight,
            (stairR+0.32f)*sinf(ang));
    }

    glEnd();


    glBegin(GL_LINE_STRIP);

    for(int i=0;i<=steps;i++)
    {
        float t=i/(float)steps;

        float ang=t*turns*2.0f*M_PI;

        glVertex3f(
            (stairR+0.32f)*cosf(ang),
            t*CYL_H+0.45f,
            (stairR+0.32f)*sinf(ang));
    }

    glEnd();

   

    for(int i=0;i<steps;i+=3)
    {
        float t=i/(float)(steps-1);

        float ang=t*turns*2.0f*M_PI;

        float x=(stairR+0.32f)*cosf(ang);
        float y=t*CYL_H;
        float z=(stairR+0.32f)*sinf(ang);

        // Vertical

        glBegin(GL_LINES);

        glVertex3f(x,y,z);
        glVertex3f(x,y+railHeight,z);

        glEnd();

        // Diagonal brace

        if(i+3<steps)
        {
            float t2=(i+3)/(float)(steps-1);

            float ang2=t2*turns*2.0f*M_PI;

            glBegin(GL_LINES);

            glVertex3f(x,y+railHeight,z);

            glVertex3f(
                (stairR+0.32f)*cosf(ang2),
                t2*CYL_H,
                (stairR+0.32f)*sinf(ang2));

            glEnd();
        }
    }
}
    void drawBottomManhole() const
    {
        const float r = 0.75f;
        const float x = CYL_R + 0.02f;
        const float y = 1.0f;
        const float z = 0.0f;

        glPushMatrix();

        glTranslatef(x, y, z);
        glRotatef(90, 0, 1, 0);

        // Cover
        glColor3f(0.55f, 0.55f, 0.55f);

        glBegin(GL_TRIANGLE_FAN);
        glNormal3f(0, 0, 1);
        glVertex3f(0, 0, 0);

        for (int i = 0; i <= 36; i++)
        {
            float a = i * 2.0f * M_PI / 36.0f;
            glVertex3f(r * cosf(a), r * sinf(a), 0);
        }

        glEnd();

        // Bolts
        glColor3f(0.20f, 0.20f, 0.20f);

        for (int i = 0; i < 12; i++)
        {
            float a = i * 2.0f * M_PI / 12.0f;

            float bx = (r - 0.10f) * cosf(a);
            float by = (r - 0.10f) * sinf(a);

            glPushMatrix();
            glTranslatef(bx, by, 0.03f);
            glutSolidSphere(0.04, 8, 8);
            glPopMatrix();
        }

        glPopMatrix();
    }

    void drawOuterWall(float yB, float yT) const
    {
        glColor3f(0.20f, 0.26f, 0.36f);
        glBegin(GL_TRIANGLE_STRIP);
        for (int i = 0; i <= SEG; i++)
        {
            float a = (float)i / SEG * 2.f * (float)M_PI;
            float c = cosf(a), s = sinf(a);
            glNormal3f(c, 0, s);
            glVertex3f(CYL_R * c, yB, CYL_R * s);
            glVertex3f(CYL_R * c, yT, CYL_R * s);
        }
        glEnd();
    }

    void drawCaps(float yB, float yT) const
    {
        glColor3f(0.15f, 0.18f, 0.26f);
        for (int cap = 0; cap < 2; cap++)
        {
            float yy = (cap == 0) ? yB : yT;
            float ny = (cap == 0) ? -1.f : 1.f;
            glBegin(GL_TRIANGLE_FAN);
            glNormal3f(0, ny, 0);
            glVertex3f(0, yy, 0);
            for (int i = 0; i <= SEG; i++)
            {
                float a = (float)(cap == 0 ? -i : i) / SEG * 2.f * (float)M_PI;
                glVertex3f(CYL_R * cosf(a), yy, CYL_R * sinf(a));
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
        for (int j = 0; j <= LAT; j++)
        {
            float yy = yB + (float)j / LAT * CYL_H;
            glBegin(GL_LINE_LOOP);
            for (int i = 0; i <= SEG; i++)
            {
                float a = (float)i / SEG * 2.f * (float)M_PI;
                glVertex3f((CYL_R + .01f) * cosf(a), yy, (CYL_R + .01f) * sinf(a));
            }
            glEnd();
        }

        // longitude lines
        for (int i = 0; i < LON; i++)
        {
            float a = (float)i / LON * 2.f * (float)M_PI;
            float c = cosf(a), s = sinf(a);
            glBegin(GL_LINES);
            glVertex3f((CYL_R + .01f) * c, yB, (CYL_R + .01f) * s);
            glVertex3f((CYL_R + .01f) * c, yT, (CYL_R + .01f) * s);
            glEnd();
        }
        glPopAttrib();
    }
};