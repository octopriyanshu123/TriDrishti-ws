// ============================================================
//  TFNode.cpp
//  One node in the robot TF tree.
//
//  Each node stores:
//    name          – frame name (for HUD / debugging)
//    tx,ty,tz      – static translation from parent frame
//    roll,pitch,yaw– static rotation  from parent frame (deg, XYZ Euler)
//    jx,jy,jz      – joint axis (unit vector, in this node's local frame)
//    jAngle        – dynamic joint angle (deg) – updated every frame
//
//  apply() pushes this node's full transform onto the GL matrix stack.
//  The caller is responsible for glPushMatrix / glPopMatrix around apply().
//
//  drawAxes() draws the frame axes AT this node's origin AFTER apply().
// ============================================================
#include <GL/glut.h>
#include <cmath>
#include <cstring>

struct TFNode
{
    // ── identity ─────────────────────────────────────────────
    char name[32];

    // ── static transform (fixed, set at construction) ────────
    float tx = 0.f, ty = 0.f, tz = 0.f;   // translation from parent
    float roll  = 0.f;   // rotation around X (deg)
    float pitch = 0.f;   // rotation around Y (deg)
    float yaw   = 0.f;   // rotation around Z (deg)

    // ── dynamic joint (changes every frame) ──────────────────
    float jx = 0.f, jy = 0.f, jz = 1.f;   // joint axis  (default = Z)
    float jAngle = 0.f;                     // joint angle (deg)

    // ── constructor ──────────────────────────────────────────
    TFNode() { name[0] = '\0'; }

    TFNode(const char* n,float tx_, float ty_, float tz_,float roll_, float pitch_, float yaw_,float jx_=0, float jy_=0, float jz_=1)
        : tx(tx_), ty(ty_), tz(tz_)
        , roll(roll_), pitch(pitch_), yaw(yaw_)
        , jx(jx_), jy(jy_), jz(jz_)
        , jAngle(0.f)
    {
        strncpy(name, n, 31);
        name[31] = '\0';
    }

    // ── apply full transform to GL matrix stack ───────────────
    //  Order: translate → roll(X) → pitch(Y) → yaw(Z) → joint
    void apply() const
    {
        glTranslatef(tx, ty, tz);
        if(roll  != 0.f) glRotatef(roll,  1, 0, 0);
        if(pitch != 0.f) glRotatef(pitch, 0, 1, 0);
        if(yaw   != 0.f) glRotatef(yaw,   0, 0, 1);
        if(jAngle != 0.f) glRotatef(jAngle, jx, jy, jz);
    }

    // ── apply ONLY the dynamic joint rotation ─────────────────
    void applyJoint() const
    {
        if(jAngle != 0.f) glRotatef(jAngle, jx, jy, jz);
    }

    // ── draw TF axes at this node's origin ───────────────────
    //   X=red  Y=green  Z=blue
    void drawAxes(float scale) const
    {
        glPushAttrib(GL_LIGHTING_BIT);
        glDisable(GL_LIGHTING);

        glColor3f(1.f, 0.15f, 0.15f);                        // X red
        glPushMatrix(); glRotatef(90,0,1,0);
        drawArrow(scale); glPopMatrix();

        glColor3f(0.15f, 1.f, 0.15f);                        // Y green
        glPushMatrix(); glRotatef(-90,1,0,0);
        drawArrow(scale); glPopMatrix();

        glColor3f(0.2f, 0.45f, 1.f);                         // Z blue
        drawArrow(scale);

        // frame name label
        glColor3f(0.9f, 0.9f, 0.5f);
        glRasterPos3f(scale*0.15f, scale*0.15f, scale*0.15f);
        for(const char* p = name; *p; p++)
            glutBitmapCharacter(GLUT_BITMAP_8_BY_13, *p);

        glPopAttrib();
    }

private:
    static void drawArrow(float len)
    {
        GLUquadric* q = gluNewQuadric();
        gluCylinder(q, len*0.04f, len*0.04f, len*0.76f, 8, 1);
        glPushMatrix();
          glTranslatef(0, 0, len*0.76f);
          gluCylinder(q, len*0.10f, 0.f, len*0.24f, 8, 1);
        glPopMatrix();
        gluDeleteQuadric(q);
    }
};