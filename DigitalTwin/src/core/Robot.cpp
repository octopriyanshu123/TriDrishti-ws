
#include "Wheel.cpp"
#include <GL/glut.h>
#include <cmath>
#include "Base.cpp"      // class Base
#include "TFNode.cpp"    // struct TFNode

class Robot
{
public:
    // ── geometry constants ───────────────────────────────────
    static constexpr float AXLE_L    = 0.99f;
    static constexpr float AXLE_R    = 0.03f;
    static constexpr float HALF_SPAN = AXLE_L * 0.5f + Wheel::TYRE_T * 0.5f;
    static constexpr float WHEELBASE = AXLE_L + Wheel::TYRE_T;

    // ── geometry parts ───────────────────────────────────────
    Wheel leftWheel  { false };
    Wheel rightWheel { true  };
    Base  base;

    // ── TF tree nodes ────────────────────────────────────────
    //  robot_base_link is computed dynamically (surface transform)
    //  and applied in draw() before walking the tree.
    //
    //  Static offsets baked from the dimension sketch:
    //    left_wheel  at z = -HALF_SPAN = -0.595, yaw flipped 180°
    //    right_wheel at z = +HALF_SPAN = +0.595, no flip
    //    base_link   at origin, no rotation
    //    axle_link   at origin, no rotation
    TFNode tf_base       { "base_link",        0,0,0,            0,0,0       };
    TFNode tf_leftWheel  { "left_wheel_link",  0,0,-HALF_SPAN,   0,0,0,     0,0,1 };
    TFNode tf_rightWheel { "right_wheel_link", 0,0, HALF_SPAN,   0,0,0,     0,0,1 };

    // ── robot pose (set by kinematics, used by surface transform) ──
    float theta  = 0.f;   // azimuth around tank (rad)
    float height = 0.f;   // height on tank wall
    float phi    = 0.f;   // heading in tangent plane (rad)

    // ── interface ────────────────────────────────────────────
    void setSpeed(float leftDeg, float rightDeg)
    {
        leftWheel.setSpeed(leftDeg);
        rightWheel.setSpeed(rightDeg);
    }
    void stop()  { leftWheel.stop();  rightWheel.stop(); }

    void reset()
    {
        theta = height = phi = 0.f;
        leftWheel.reset();
        rightWheel.reset();
        tf_leftWheel.jAngle  = 0.f;
        tf_rightWheel.jAngle = 0.f;
    }

    // advance kinematics one frame
    void update(float cylR, float cylH)
    {
        const float toRad = (float)M_PI / 180.f;
        float dL = leftWheel.speedDeg  * toRad * Wheel::TYRE_R;
        float dR = rightWheel.speedDeg * toRad * Wheel::TYRE_R;

        float arc   = (dL + dR) * 0.5f;
        float omega = (dR - dL) / WHEELBASE;

        phi    += omega;
        theta  += arc * cosf(phi) / cylR;
        height += arc * sinf(phi);

        const float PI2 = 2.f * (float)M_PI;
        if(theta >  (float)M_PI) theta -= PI2;
        if(theta < -(float)M_PI) theta += PI2;

        float maxH = cylH*0.5f - Wheel::TYRE_R - 0.02f;
        if(height >  maxH) height =  maxH;
        if(height < -maxH) height = -maxH;

        // advance wheel spins and sync to TFNode joint angles
        leftWheel.update();
        rightWheel.update();
        tf_leftWheel.jAngle  = leftWheel.spinDeg;
        tf_rightWheel.jAngle = rightWheel.spinDeg;
    }

    // ── draw: walk TF tree ───────────────────────────────────
    void draw(float cylR) const
    {
        // ── robot_base_link ──────────────────────────────────
        glPushMatrix();
        applySurfaceTransform(cylR);      // dynamic: pose on cylinder
        // drawBaseAxes();                   // robot_base_link TF axes

        // ── base_link ────────────────────────────────────────
        glPushMatrix();
        // tf_base.apply();
        base.draw();
        // tf_base.drawAxes(0.6f);        // uncomment to show base_link axes
        glPopMatrix();

        // ── axle_link ────────────────────────────────────────
        glPushMatrix();
        // drawAxleGeometry();
        // tf_axle.drawAxes(0.5f);        // uncomment to show axle_link axes
        glPopMatrix();

        // ── left_wheel_link ──────────────────────────────────
        glPushMatrix();
        tf_leftWheel.apply();             // static offset + yaw180 + Z-spin
        leftWheel.draw(0);
        // tf_leftWheel.drawAxes(0.55f);     // left wheel TF axes
        glPopMatrix();

        // ── right_wheel_link ─────────────────────────────────
        glPushMatrix();
        tf_rightWheel.apply();            // static offset + Z-spin
        rightWheel.draw(1);
        // tf_rightWheel.drawAxes(0.55f);    // right wheel TF axes
        glPopMatrix();

        glPopMatrix();  // end robot_base_link
    }

private:
    void applySurfaceTransform(float cylR) const
    {
        const float toDeg = 180.f / (float)M_PI;
        const float R = cylR + Wheel::TYRE_R;

        glTranslatef(R*cosf(theta), height, R*sinf(theta));  // 1
        glRotatef(theta * toDeg,  0, 1, 0);                   // 2
        glRotatef(90.f,           0, 1, 0);                   // 3
        glRotatef(90.f,           1, 0, 0);                   // 4
        glRotatef(phi * toDeg,    0, 0, 1);                   // 5
    }

    // ── robot_base_link axes ─────────────────────────────────
    void drawBaseAxes() const
    {
        glPushAttrib(GL_LIGHTING_BIT);
        glDisable(GL_LIGHTING);
        // X red
        glColor3f(1.f,0.15f,0.15f);
        glPushMatrix(); glRotatef(90,0,1,0); drawArrow(1.4f); glPopMatrix();
        // Y green
        glColor3f(0.15f,1.f,0.15f);
        glPushMatrix(); glRotatef(-90,1,0,0); drawArrow(1.4f); glPopMatrix();
        // Z blue
        glColor3f(0.2f,0.45f,1.f);
        drawArrow(1.4f);
        // label
        glColor3f(0.9f,0.9f,0.5f);
        glRasterPos3f(0.15f,0.15f,0.15f);
        const char* lbl = "robot_base_link";
        for(const char* p=lbl;*p;p++) glutBitmapCharacter(GLUT_BITMAP_8_BY_13,*p);
        glPopAttrib();
    }

    // ── axle rod geometry ────────────────────────────────────
    void drawAxleGeometry() const
    {
        glColor3f(0.55f, 0.55f, 0.58f);
        glPushMatrix();
        glTranslatef(0, 0, -AXLE_L*0.5f);
        drawCylZ(AXLE_R, AXLE_L*0.5f, 16);
        glPopMatrix();
    }

    // ── helpers ──────────────────────────────────────────────
    static void drawArrow(float len)
    {
        GLUquadric* q = gluNewQuadric();
        gluCylinder(q, len*0.04f, len*0.04f, len*0.76f, 8, 1);
        glPushMatrix();
          glTranslatef(0,0,len*0.76f);
          gluCylinder(q, len*0.10f, 0.f, len*0.24f, 8, 1);
        glPopMatrix();
        gluDeleteQuadric(q);
    }

    static void drawCylZ(float r, float halfT, int seg)
    {
        glBegin(GL_TRIANGLE_STRIP);
        for(int i=0;i<=seg;i++){
            float a=(float)i/seg*2.f*(float)M_PI;
            float c=cosf(a),s=sinf(a);
            glNormal3f(c,s,0);
            glVertex3f(r*c,r*s,-halfT);
            glVertex3f(r*c,r*s, halfT);
        }
        glEnd();
        for(int side=-1;side<=1;side+=2){
            float zz=(float)side*halfT;
            glBegin(GL_TRIANGLE_FAN);
            glNormal3f(0,0,(float)side);
            glVertex3f(0,0,zz);
            for(int i=0;i<=seg;i++){
                float a=(float)(side*i)/seg*2.f*(float)M_PI;
                glVertex3f(r*cosf(a),r*sinf(a),zz);
            }
            glEnd();
        }
    }
};