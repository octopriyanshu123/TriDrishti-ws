// ============================================================
//  FlatRobot.cpp
//  Differential-drive robot on a FLAT ground plane.
//  Uses the existing Wheel and Base classes unchanged.
//
//  Flat-ground frame  (standard ROS convention):
//    Y = up
//    X = forward
//    Z = left  (right-hand rule)
//
//  How Wheel fits the flat frame:
//    Wheel class spins around its own Z axis.
//    On flat ground the wheel axle must point along world Z.
//    So we place the wheel with NO extra rotation –
//    its Z axis already points sideways.
//    We only rotate 90° around Z to stand the wheel upright
//    (so the tyre touches the ground at Y=0).
//
//  How Base fits the flat frame:
//    Base class has X=outward, Y=up, Z=axle.
//    On flat ground "outward" becomes "up" (Y).
//    So we rotate the base 90° around Z:
//      Base X → world Y  (up)
//      Base Y → world -X (backward, then we flip)
//      Base Z → world Z  (left/right) stays correct
//    Net: glRotatef(-90, 0,0,1) before drawing Base.
//
//  Assembly (in robot local frame, origin at ground centre):
//    Base    : rotated -90° around Z, centred at (0, BASE_T/2, 0)
//    Left  wheel : at (0, TYRE_R, -HALF_SPAN), rotated 90° around Z
//    Right wheel : at (0, TYRE_R, +HALF_SPAN), rotated 90° around Z
//
//  Kinematics (diff-drive, flat ground):
//    arc   = (dL + dR) / 2
//    omega = (dR - dL) / WHEELBASE
//    yaw  += omega
//    x    += arc * cos(yaw)
//    z    += arc * sin(yaw)
// ============================================================
#include <GL/glut.h>
#include <cmath>
#include "Wheel.cpp"
#include "Base.cpp"

class FlatRobot
{
public:
    // ── assembly constants ───────────────────────────────────
    //   HALF_SPAN = AXLE_L/2 + TYRE_T/2
    //             = 0.99/2   + 0.20/2   = 0.595
    static constexpr float HALF_SPAN = 0.595f;
    static constexpr float WHEELBASE = 1.19f; // centre-to-centre

    // ── parts ────────────────────────────────────────────────
    Wheel leftWheel{false};
    Wheel rightWheel{true};
    Base base;

    // ── world pose ───────────────────────────────────────────
    float x = 0.f;   // world X position
    float z = 0.f;   // world Z position
    float yaw = 0.f; // heading (rad), 0 = +X direction

    // ── interface ────────────────────────────────────────────
    void setSpeed(float leftDeg, float rightDeg)
    {
        leftWheel.setSpeed(leftDeg);
        rightWheel.setSpeed(rightDeg);
    }
    void stop()
    {
        leftWheel.stop();
        rightWheel.stop();
    }

    void reset()
    {
        x = z = yaw = 0.f;
        leftWheel.reset();
        rightWheel.reset();
    }

    // advance one simulation frame
    void update()
    {
        const float toRad = (float)M_PI / 180.f;
        float dL = leftWheel.speedDeg * toRad * Wheel::TYRE_R;
        float dR = rightWheel.speedDeg * toRad * Wheel::TYRE_R;

        float arc = (dL + dR) * 0.5f;
        float omega = (dR - dL) / WHEELBASE;

        yaw += omega;
        x += arc * cosf(yaw);
        z += arc * sinf(yaw);

        leftWheel.update();
        rightWheel.update();
    }

    // ── draw ─────────────────────────────────────────────────
    void draw() const
    {
        glPushMatrix(); 
        base.draw();
        glTranslatef(0,0,0);
        glRotatef(180.f, 0, 0, 1);   // stand wheel upright
        leftWheel.draw(0);      // draws wheel HERE
        glPopMatrix();                 // RESTORE: back to Robot position

        glPopMatrix(); // RESTORE
    }

private:
    //   glRotatef(-90, 0,0,1):  Base X→Y,  Base Y→-X,  Base Z→Z
    //   then shift up by BASE_T/2 so bottom face sits on Y=0
    void drawBase() const
    {
        glPushMatrix();

        base.draw();
        glTranslatef(0.f, 0.5f, 0.f);
        glRotatef(-90.f, 0, 0, 1);
        leftWheel.draw(0);
        rightWheel.draw(1);
        glPopMatrix();
    }

    // ── Wheel: rotate 90° around X so tyre stands upright ───
    //   Wheel default: disk in XY plane, axle along Z.
    //   After RotateX(90°): disk in XZ plane, axle along -Y …
    //   We need axle along Z and tyre rim touching Y=0.
    //   Solution: RotateX(-90°) → disk stands vertical in XZ,
    //   tyre bottom at Y=0 when translated to Y=TYRE_R.
    void drawLeftWheel() const
    {
        glPushMatrix();
        glTranslatef(0.f, Wheel::TYRE_R, -HALF_SPAN);
        // glRotatef(-90.f, 1, 0, 0);   // stand wheel upright
        // glRotatef(180.f, 0, 1, 0);   // flip Z outward for left wheel
        leftWheel.draw(0);
        glPopMatrix();
    }

    void drawRightWheel() const
    {
        glPushMatrix();
        glTranslatef(0.f, Wheel::TYRE_R, HALF_SPAN);
        // glRotatef(-90.f, 1, 0, 0);   // stand wheel upright
        rightWheel.draw(1);
        glPopMatrix();
    }
};